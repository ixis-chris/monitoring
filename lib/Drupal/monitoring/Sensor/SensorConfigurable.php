<?php

/**
 * @file
 * Configurable sensor abstract class.
 */

namespace Drupal\monitoring\Sensor;

/**
 * Sensor extension providing generic functionality for custom
 * sensor settings.
 *
 * Note that settings are specific to a sensor and a service.
 */
abstract class SensorConfigurable extends Sensor implements SensorConfigurableInterface {

  /**
   * {@inheritdoc}
   */
  public function settingsForm($form, &$form_state) {

    // If sensor provides settings form, automatically provide settings to
    // enable the sensor.
    $form['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enabled'),
      '#description' => t('Check to have the sensor trigger.'),
      '#default_value' => $this->isEnabled(),
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function settingsFormValidate($form, &$form_state) {
    // Do nothing.
  }
}
