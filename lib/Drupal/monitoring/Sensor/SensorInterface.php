<?php
/**
 * @file
 * Monitoring sensor interface.
 */

namespace Drupal\monitoring\Sensor;

use Drupal\monitoring\Result\SensorResultInterface;

/**
 * Defines basic operations of a monitoring sensor.
 */
interface SensorInterface {

  /**
   * Gets sensor name (not the label).
   *
   * @return string
   *   Sensor name.
   */
  public function getSensorName();

  /**
   * Runs sensor.
   *
   * Within this method the sensor status and value needs to be set.
   *
   * @param SensorResultInterface $sensor_result
   *   Sensor result object.
   */
  public function runSensor(SensorResultInterface $sensor_result);

  /**
   * Determines if sensor is enabled.
   *
   * @return boolean
   *   Enabled flag.
   */
  public function isEnabled();

}
