<?php

use Drupal\monitoring\Result\SensorResultInterface;

/**
 * Implements hook_monitoring_sensor_info().
 */
function monitoring_test_monitoring_sensor_info() {
  return array(
    'test_sensor_custom' => array(
      'label' => 'Test from custom hook',
      'description' => '',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => FALSE,
        'category' => 'Test',
      ),
    ),
    'watchdog_aggregate_test' => array(
      'label' => 'Watchdog aggregate test',
      'description' => 'Test sensor for watchdog aggregate.',
      'sensor class' => 'Drupal\monitoring\Sensor\Sensors\SensorDatabaseAggregator',
      'settings' => array(
        'log_calls' => FALSE,
        'category' => 'Test',
        'thresholds' => array(
          SensorResultInterface::STATUS_CRITICAL => array('exceeds' => 2),
          SensorResultInterface::STATUS_WARNING => array('exceeds' => 1),
        ),
        'table' => 'watchdog',
        'conditions' => array(
          array('field' => 'type', 'value' => 'test_watchdog_aggregate_sensor')
        ),
      ),
    ),
    'test_sensor' => array(
      'label' => 'Test sensor',
      'description' => 'Test sensor status',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => TRUE,
        'categories' => 'Test',
      ),
    ),
    'test_sensor_inner' => array(
      'label' => 'Test sensor inner',
      'description' => 'Test sensor that sets inner error intervals',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => TRUE,
        'categories' => 'Test',
        'thresholds' => array(
          SensorResultInterface::STATUS_CRITICAL => array('inner_interval' => array(3, 6)),
          SensorResultInterface::STATUS_WARNING => array('inner_interval' => array(1, 8)),
        ),
      ),
    ),
    'test_sensor_outer' => array(
      'label' => 'Test sensor outer',
      'description' => 'Test sensor that sets outer error intervals',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => TRUE,
        'category' => 'Test',
        'thresholds' => array(
          SensorResultInterface::STATUS_CRITICAL => array('outer_interval' => array(10, 50)),
          SensorResultInterface::STATUS_WARNING => array('outer_interval' => array(20, 40)),
        ),
      ),
    ),
    'test_sensor_exceeds' => array(
      'label' => 'Test sensor exceeds',
      'description' => 'Test sensor that sets error interval above give value',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => TRUE,
        'category' => 'Test',
        'thresholds' => array(
          SensorResultInterface::STATUS_CRITICAL => array('exceeds' => 10),
          SensorResultInterface::STATUS_WARNING => array('exceeds' => 5),
        ),
      ),
    ),
    'test_sensor_falls' => array(
      'label' => 'Test sensor falls',
      'description' => 'Test sensor that sets error interval below give value',
      'sensor class' => 'Drupal\monitoring_test\Sensor\Sensors\TestSensor',
      'settings' => array(
        'log_calls' => TRUE,
        'category' => 'Test',
        'thresholds' => array(
          SensorResultInterface::STATUS_CRITICAL => array('falls' => 5),
          SensorResultInterface::STATUS_WARNING => array('falls' => 10),
        ),
      ),
    ),
  );
}
