<?php
/**
 * @file
 * Contains \Drupal\monitoring\Result\SensorResultInterface.
 */

namespace Drupal\monitoring\Result;

use Drupal\monitoring\Sensor\SensorInfo;

/**
 * Container for sensor result.
 */
interface SensorResultInterface {

  const STATUS_OK = 'OK';
  const STATUS_INFO = 'INFO';
  const STATUS_WARNING = 'WARNING';
  const STATUS_CRITICAL = 'CRITICAL';
  const STATUS_UNKNOWN = 'UNKNOWN';

  /**
   * Gets sensor status.
   *
   * This method should be called after runSensor().
   *
   * @return string
   *   Sensor status.
   */
  function getSensorStatus();

  /**
   * Sets sensor status.
   *
   * @param string $status
   *   One of SensorResultInterface::STATUS_* constants.
   */
  function setSensorStatus($status);

  /**
   * Gets compiled sensor status message.
   *
   * @return string
   *   Sensor status message.
   */
  function getSensorMessage();

  /**
   * Sets the result message.
   *
   * As opposed to addResultMessage() this sets the only message and removes any
   * messages previously added.
   *
   * @param string $message
   *   Message to be set.
   * @param array $variables
   *   Dynamic values to be replaced for placeholders in the message.
   */
  function setSensorMessage($message, array $variables = array());

  /**
   * Adds sensor status message.
   *
   * @param string $message
   *   Message to be set.
   * @param array $variables
   *   Dynamic values to be replaced for placeholders in the message.
   */
  function addSensorStatusMessage($message, array $variables = array());

  /**
   * Will compile added messages and deal with status.
   */
  function compile();

  /**
   * Gets the sensor metric value.
   *
   * @return mixed
   *   Whatever value the sensor is supposed to return.
   */
  function getSensorValue();

  /**
   * Sets sensor value.
   *
   * @param mixed $value
   */
  function setSensorValue($value);

  /**
   * Gets the sensor expected value.
   *
   * @return mixed
   *   Whatever value the sensor is supposed to return.
   */
  function getSensorExpectedValue();

  /**
   * Sets sensor expected value.
   *
   * Set to NULL if you want to prevent the default sensor result assessment.
   * Use 0/FALSE values instead.
   *
   * In case an interval is expected, do not set the expected value, thresholds
   * are used instead.
   *
   * @param mixed $value
   */
  function setSensorExpectedValue($value);

  /**
   * Get sensor execution time.
   *
   * @return double
   */
  function getSensorExecutionTime();

  /**
   * Sets sensor execution time.
   *
   * @param double $time
   *   Sensor execution time.
   */
  function setSensorExecutionTime($time);

  /**
   * Casts/processes the sensor value into numeric representation.
   *
   * @return number
   *   Numeric sensor value.
   */
  function toNumber();

  /**
   * Determines if data for given result object are cached.
   *
   * @return boolean
   *   Cached flag.
   */
  function isCached();

  /**
   * The result data timestamp.
   *
   * @return int
   *   Unix timestamp.
   */
  function getTimestamp();

  /**
   * Gets sensor data ready to be used for entity creation.
   *
   * @return array
   *   Entity values.
   */
  function getEntityValues();

  /**
   * Provides verbose info about the sensor result.
   *
   * @param bool $as_array
   *   Flag if to output the verbose result as array.
   *
   * @return string
   *   Verbose info.
   */
  function verbose($as_array = FALSE);

  /**
   * Gets sensor name.
   *
   * @return string
   */
  public function getSensorName();

  /**
   * Gets sensor info.
   *
   * @return SensorInfo
   */
  public function getSensorInfo();

  /**
   * Checks if sensor is in UNKNOWN state.
   *
   * @return boolean
   */
  public function isUnknown();

  /**
   * Checks if sensor is in WARNING state.
   *
   * @return boolean
   */
  public function isWarning();

  /**
   * Checks if sensor is in CRITICAL state.
   *
   * @return boolean
   */
  public function isCritical();

  /**
   * Checks if sensor is in OK state.
   *
   * @return boolean
   */
  public function isOk();
}
