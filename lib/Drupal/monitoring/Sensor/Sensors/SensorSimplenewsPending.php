<?php
/**
 * @file
 * Contains Drupal\monitoring\Sensor\Sensors\SensorSimplenewsPending
 */

namespace Drupal\monitoring\Sensor\Sensors;

use Drupal\monitoring\Result\SensorResultInterface;
use Drupal\monitoring\Sensor\SensorThresholds;

/**
 * Monitors pending items in the simplenews spool.
 */
class SensorSimplenewsPending extends SensorThresholds {

  /**
   * {@inheritdoc}
   */
  public function runSensor(SensorResultInterface $result) {
    module_load_include('inc', 'simplenews', 'includes/simplenews.mail');
    $result->setSensorValue(simplenews_count_spool(array('status' => SIMPLENEWS_SPOOL_PENDING)));
  }
}
