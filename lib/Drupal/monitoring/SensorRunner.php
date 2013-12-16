<?php
/**
 * @file
 * Contains Drupal\monitoring\SensorRunner
 */

namespace Drupal\monitoring;

use Drupal\monitoring\Result\SensorResultInterface;
use Drupal\monitoring\Sensor\SensorInfo;

/**
 * Sensor runner helper class to instantiate and run requested sensors.
 */
class SensorRunner implements \IteratorAggregate {

  /**
   * Internal sensor result cache.
   *
   * @var array
   */
  protected $cache = array();

  /**
   * List of sensors info keyed by sensor name that are meant to run.
   *
   * @var \Drupal\monitoring\Sensor\SensorInfo[]
   */
  protected $sensors = array();

  /**
   * Flag to force sensor run.
   *
   * @var bool
   */
  protected $forceRun = FALSE;

  /**
   * Flag to switch the collecting of verbose output.
   *
   * @var bool
   */
  protected $verbose = FALSE;

  /**
   * Captured verbose output.
   *
   * @var string
   */
  protected $verboseOutput = '';

  /**
   * Result logging mode.
   *
   * @var bool
   */
  protected $loggingMode = FALSE;

  /**
   * Constructor.
   *
   * @param array $sensors
   *   Associative array of sensor names => sensor info.
   */
  public function __construct(array $sensors = array()) {
    $this->sensors = $sensors;
    if (empty($sensors)) {
      $this->sensors = monitoring_sensor_info_instance();
    }
    $this->loadCache();
  }

  /**
   * Forces to run sensors even there is cached data available.
   *
   * @param bool $force
   *   Whether or not to force a sensor to run. Defaults to TRUE.
   */
  public function forceRun($force = TRUE) {
    $this->forceRun = $force;
  }

  /**
   * Sets flag to collect verbose info.
   *
   * @param bool $verbose
   *   Verbose flag.
   */
  public function verbose($verbose = TRUE) {
    $this->verbose = $verbose;
  }

  /**
   * Sets result logging mode.
   *
   * @param string $mode
   *   (NULL, on_request, all)
   */
  public function setLoggingMode($mode) {
    $this->loggingMode = $mode;
  }

  /**
   * Loads available sensor results from cache.
   */
  public function loadCache() {
    $cids = array();

    // Only load sensor caches if they define caching.
    foreach ($this->sensors as $name => $sensor_info) {
      if ($sensor_info->getCachingTime()) {
        $cids[] = $this->getSensorCid($name);
      }
    }
    if ($cids) {
      foreach (cache_get_multiple($cids) as $cache) {
        if ($cache->expire > REQUEST_TIME) {
          $this->cache[$cache->data['name']] = $cache->data;
        }
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getIterator() {
    $results = $this->runSensors();
    return new \ArrayIterator($results);
  }

  /**
   * Runs sensors.
   *
   * @return array<SensorResultInterface>
   *   Array of sensor results.
   */
  public function runSensors() {
    $results = array();

    foreach ($this->sensors as $name => $info) {
      if ($result = $this->runSensor($info)) {
        $results[$name] = $result;
      }
    }
    $this->saveResults($results);
    $this->cacheResults($results);
    return $results;
  }

  /**
   * Main runner method.
   *
   * @param SensorInfo $sensor_info
   *   Sensor info
   *
   * @return SensorResultInterface
   *   Sensor result.
   */
  protected function runSensor(SensorInfo $sensor_info) {
    $sensor = $this->getSensorObject($sensor_info);

    // Check if sensor is enabled.
    if (!$sensor->isEnabled()) {
      return NULL;
    }

    $result = $this->getResultObject($sensor_info);

    // In case result is not yet cached run sensor.
    if (!$result->isCached()) {

      timer_start($sensor_info->getName());
      $sensor->runSensor($result);
      $timer = timer_stop($sensor_info->getName());

      $result->setSensorExecutionTime($timer['time']);
      $result->compile();
    }

    // Capture verbose output if requested and if we are able to do so.
    if ($this->verbose && $sensor_info->isExtendedInfo()) {
      $this->verboseOutput[$sensor_info->getName()] = $sensor->resultVerbose($result);
    }

    return $result;
  }

  /**
   * Helper method to save results into result log.
   *
   * @param \Drupal\monitoring\Result\SensorResultInterface[] $results
   *   Results to be saved.
   */
  protected function saveResults(array $results) {
    foreach ($results as $result) {
      // Skip if the result is cached.
      if ($result->isCached()) {
        continue;
      }

      // Define which sensor categories should not be logged by watchdog.
      $skip_categories = array('Past', 'Watchdog');
      // If we have critical log the event into using watchdog.
      if ($result->isCritical() && (!in_array($result->getSensorInfo()->getCategory(), $skip_categories))) {
        watchdog('monitoring', 'Failing sensor %name with message %message and value %value', array(
          '%name' => $result->getSensorInfo()->getLabel(),
          '%message' => $result->getSensorMessage(),
          '%value' => $result->getSensorValue(),
        ), WATCHDOG_ERROR);
      }

      $old_status = NULL;
      if ($last_result = monitoring_sensor_result_last($result->getSensorName())) {
        $old_status = $last_result->sensor_status;
      }

      if ($result->getSensorInfo()->logResults($this->loggingMode, $old_status, $result->getSensorStatus())) {
        monitoring_sensor_result_save($result);
      }
    }
  }

  /**
   * Helper method to cache results.
   *
   * @param \Drupal\monitoring\Result\SensorResultInterface[] $results
   *   Results to be cached.
   */
  protected function cacheResults(array $results) {
    // @todo: Cache in a single array, with per item expiration?
    foreach ($results as $result) {
      $definition = $result->getSensorInfo();
      if ($definition->getCachingTime() && !$result->isCached()) {
        $data = $result->getEntityValues();
        $data['name'] = $result->getSensorName();
        cache_set($this->getSensorCid($result->getSensorName()), $data, 'cache', REQUEST_TIME + $definition->getCachingTime());
      }
    }
  }

  /**
   * Instantiates sensor object.
   *
   * @param SensorInfo $sensor_info
   *   Sensor info.
   *
   * @return \Drupal\monitoring\Sensor\SensorInterface
   *   Instantiated sensor.
   */
  protected function getSensorObject(SensorInfo $sensor_info) {
    $sensor_class = $sensor_info->getSensorClass();
    $sensor = new $sensor_class($sensor_info);
    return $sensor;
  }

  /**
   * Instantiates sensor result object.
   *
   * @param SensorInfo $sensor_info
   *   Sensor info.
   *
   * @return \Drupal\monitoring\Result\SensorResultInterface
   *   Instantiated sensor result object.
   */
  protected function getResultObject(SensorInfo $sensor_info) {
    $result_class = $sensor_info->getResultClass();

    if (!$this->forceRun && isset($this->cache[$sensor_info->getName()])) {
      $result = new $result_class($sensor_info, $this->cache[$sensor_info->getName()]);
    }
    else {
      $result = new $result_class($sensor_info);
    }
    return $result;
  }

  /**
   * Gets sensor cache id.
   *
   * @param string $sensor_name
   *   The name of the sensor.
   *
   * @return string
   *   Cache id.
   */
  public function getSensorCid($sensor_name) {
    return 'monitoring_sensor_result:' . $sensor_name;
  }

  /**
   * Gets verbose output after sensors run.
   *
   * @return string
   *   Verbose output.
   */
  public function verboseOutput() {
    return $this->verboseOutput;
  }

}
