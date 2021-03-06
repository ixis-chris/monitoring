<?php
/**
 * @file
 * Admin page/form callbacks.
 */

use Drupal\monitoring\SensorRunner;
use Drupal\monitoring\Sensor\Sensor;
use Drupal\monitoring\Sensor\SensorConfigurableInterface;
use Drupal\monitoring\Result\SensorResultInterface;
use Drupal\monitoring\Sensor\SensorInfo;

function monitoring_config_sensors_overview_form($form, &$form_state) {
  $options = array();
  $default_value = array();

  /** @var SensorInfo $sensor_info */
  foreach (monitoring_sensor_info_instance() as $sensor_name => $sensor_info) {
    $row = array(
      'label' => $sensor_info->getLabel(),
      'description' => $sensor_info->getDescription(),
    );

    $row['status'] = $sensor_info->isEnabled() ? t('Enabled') : t('Disabled');

    $links = array();
    if ($sensor_info->isConfigurable()) {
      $links[] = array('title' => t('Settings'), 'href' => 'admin/config/system/monitoring/sensors/' . $sensor_name,
        'query' => array('destination' => 'admin/config/system/monitoring/sensors'));
    }

    $row['actions'] = '';
    if (!empty($links)) {
      $row['actions'] = theme('links', array('links' => $links));
    }

    $options[$sensor_name] = $row;
    $default_value[$sensor_name] = $sensor_info->isEnabled();
  }

  $header = array(
    'label' => t('Sensor name'),
    'description' => t('Description'),
    'status' => t('Status'),
    'actions' => t('Actions'),
  );

  $form['sensors'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#default_value' => $default_value,
    '#attributes' => array(
      'id' => 'monitoring-sensors-config-overview',
    ),
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function monitoring_config_sensors_overview_form_submit($form, &$form_state) {
  foreach ($form_state['values']['sensors'] as $sensor_name => $enabled) {
    $settings = monitoring_sensor_settings_get($sensor_name);
    $settings['enabled'] = (boolean) $enabled;
    monitoring_sensor_settings_save($sensor_name, $settings);
  }
}

/**
 * Page callback to output list of sensors.
 *
 * @return array
 *   Renderable array.
 */
function monitoring_reports_sensors_overview() {

  $rows = array();
  $runner = new SensorRunner();
  $results = $runner->runSensors();
  $status_overview = array(
    SensorResultInterface::STATUS_OK => 0,
    SensorResultInterface::STATUS_INFO => 0,
    SensorResultInterface::STATUS_WARNING => 0,
    SensorResultInterface::STATUS_CRITICAL => 0,
    SensorResultInterface::STATUS_UNKNOWN => 0,
  );
  $total_execution_time = 0;
  $non_cached_execution_time = 0;

  foreach (monitoring_sensor_info_by_categories() as $category => $category_sensor_info) {

    // Category grouping row.
    $rows[] = array(
      'data' => array(
        'label' => array(
          'data' => '<h3>' . $category . '</h3>',
          'colspan' => 7
        ),
      ),
    );
    $ok_row_count = 0;

    foreach ($category_sensor_info as $sensor_name => $info) {
      if (!isset($results[$sensor_name])) {
        continue;
      }
      /** @var Drupal\monitoring\Result\SensorResultInterface $sensor_result */
      $sensor_result = $results[$sensor_name];

      $sensor_info = monitoring_sensor_info_instance($sensor_name);

      $row['data']['label'] = '<span title="' . $sensor_info->getDescription() . '">' . $sensor_info->getLabel() . '</span>';

      $row['data']['sensor_status'] = array(
        'data' => $sensor_result->getSensorStatus(),
        'class' => array('status'),
      );

      $row['data']['timestamp'] = format_interval(REQUEST_TIME - $sensor_result->getTimestamp());
      $row['data']['execution_time'] = array(
        'data' => $sensor_result->getSensorExecutionTime() . 'ms',
        'class' => array('execution-time'),
      );
      $row['data']['sensor_status_message'] = truncate_utf8(strip_tags($sensor_result->getSensorMessage()), 200, TRUE, TRUE);

      $row['class'] = array('monitoring-' . strtolower($sensor_result->getSensorStatus()));

      $links = array();
      $links['details'] = array('title' => t('Details'), 'href' => 'admin/reports/monitoring/sensors/' . $sensor_name);
      if (user_access('monitoring verbose')) {
        $links['log'] = array('title' => t('Log'), 'href' => 'admin/reports/monitoring/sensors/' . $sensor_name . '/log');
      }
      if ($sensor_info->isConfigurable() && user_access('administer monitoring')) {
        $links['settings'] = array('title' => t('Settings'), 'href' => 'admin/config/system/monitoring/sensors/' . $sensor_name,
          'query' => array('destination' => 'admin/reports/monitoring'));
      }

      drupal_alter('monitoring_sensor_links', $links, $sensor_info);

      $row['data']['actions'] = array();
      if (!empty($links)) {
        $row['data']['actions']['data'] = theme('links__ctools_dropbutton',
          array('links' => $links, 'attributes' => array('class' => array('links', 'inline'))));
      }

      $rows[] = $row;

      $status_overview[$sensor_result->getSensorStatus()]++;
      $total_execution_time += $sensor_result->getSensorExecutionTime();
      if (!$sensor_result->isCached()) {
        $non_cached_execution_time += $sensor_result->getSensorExecutionTime();
      }
      if ($sensor_result->getSensorStatus() == SensorResultInterface::STATUS_OK) {
        $ok_row_count++;
      }
      else {
        $ok_row_count = -1;
      }
    }

    // Add special class if all sensors of a category are ok.
    if ($ok_row_count >= 0) {
      $index = count($rows) - $ok_row_count - 1;
      $rows[$index]['class'][] = 'sensor-category-ok';
    }
  }

  $output['summary'] = array(
    '#theme' => 'monitoring_overview_summary',
    '#status_overview' => $status_overview,
    '#total_execution_time' => $total_execution_time,
    '#non_cached_execution_time' => $non_cached_execution_time,
  );

  $header = array(
    t('Sensor name'),
    array('data' => t('Status'), 'class' => array('status')),
    t('Called before'),
    t('Execution time'),
    t('Status Message'),
    array('data' => t('Actions'), 'class' => array('actions')),
  );

  $monitoring_error_sensors = $status_overview[SensorResultInterface::STATUS_WARNING] +
      $status_overview[SensorResultInterface::STATUS_CRITICAL] +
      $status_overview[SensorResultInterface::STATUS_UNKNOWN];

  $output['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#sticky' => TRUE,
    '#attributes' => array(
      'class' => array('monitoring-severity-colors'),
      'id' => 'monitoring-sensors-overview',
    ),
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'monitoring') . '/monitoring.css',
        // Yes, we must include all these to get that ctools dropbutton look and
        // work properly.
        drupal_get_path('module', 'ctools') . '/css/dropbutton.css',
        drupal_get_path('module', 'views') . '/css/views-admin.contextual.css',
        drupal_get_path('module', 'views') . '/css/views-admin.ctools.css',
        drupal_get_path('module', 'views') . '/css/views-admin.seven.css',
      ),
      'js' => array(
        array(
          'data' => drupal_get_path('module', 'monitoring') . '/monitoring.js',
          'type' => 'file',
        ),
        array(
          'data' => array('monitoring_error_sensors' => $monitoring_error_sensors),
          'type' => 'setting',
        ),
      ),
    ),
  );

  return $output;
}

/**
 * Service sensors setting form.
 */
function monitoring_sensor_settings_form($form, &$form_state, $sensor_name) {

  $sensor_info = monitoring_sensor_info_instance($sensor_name);
  if (!$sensor_info->isConfigurable()) {
    return $form;
  }

  $sensor_class = $sensor_info->getSensorClass();
  /** @var SensorConfigurableInterface $sensor */
  $sensor = new $sensor_class($sensor_info);
  // Note that here we cannot set the sensor object as some sensor objects may
  // carry with itself PDOStatements that cannot be serialised.
  $form_state['sensor_name'] = $sensor_name;

  $form_key = monitoring_sensor_settings_key($sensor_name);

  $form[$form_key] = array(
    '#tree' => TRUE,
  );

  $form[$form_key] = $sensor->settingsForm($form[$form_key], $form_state);

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function monitoring_sensor_settings_form_validate($form, &$form_state) {
  /** @var SensorConfigurableInterface $sensor */
  $sensor_info = monitoring_sensor_info_instance($form_state['sensor_name']);
  $sensor_class = $sensor_info->getSensorClass();
  $sensor = new $sensor_class($sensor_info);
  $sensor->settingsFormValidate($form, $form_state);
}

function monitoring_sensor_settings_form_submit($form, &$form_state) {
  $form_key = monitoring_sensor_settings_key($form_state['sensor_name']);
  monitoring_sensor_settings_save($form_state['sensor_name'], $form_state['values'][$form_key]);
  drupal_set_message(t('Sensor settings saved.'));
}

function monitoring_sensor_details($form, &$form_state, $sensor_name) {
  $form_state['sensor_name'] = $sensor_name;
  $sensor_info = monitoring_sensor_info_instance($sensor_name);

  $verbose = array();
  $result = monitoring_sensor_run($sensor_name, FALSE, $verbose);

  $form['force_run'] = array(
    '#type' => 'submit',
    '#value' => t('Force run'),
    '#access' => user_access('monitoring force run'),
    '#weight' => -15,
  );

  $form['sensor_result'] = array(
    '#type' => 'fieldset',
    '#title' => t('Result'),
    '#weight' => -10,
  );

  $form['sensor_result']['cached'] = array(
    '#type' => 'item',
    '#title' => t('Cached'),
    '#markup' => $result->isCached() ? format_date($result->getTimestamp()) : t('no')
  );
  $form['sensor_result']['value'] = array(
    '#type' => 'item',
    '#title' => t('Value'),
    '#markup' => $result->getSensorValue(),
  );
  if ($result->getSensorExpectedValue() !== NULL) {
    $form['sensor_result']['expected_value'] = array(
      '#type' => 'item',
      '#title' => t('Expected value'),
      '#markup' => $sensor_info->isDefiningThresholds() ?
          t('Thresholds type %type <pre>@intervals</pre>',
            array('%type' => $sensor_info->getThresholdsType(), '@intervals' => var_export($sensor_info->getThresholdsIntervals(), TRUE))) :
          $result->getSensorExpectedValue(),
    );
  }
  $form['sensor_result']['status'] = array(
    '#type' => 'item',
    '#title' => t('Status'),
    '#markup' => $result->getSensorStatus(),
  );
  $form['sensor_result']['message'] = array(
    '#type' => 'item',
    '#title' => t('Message'),
    '#markup' => $result->getSensorMessage(),
  );

  if (isset($verbose[$sensor_name])) {
    $form['verbose'] = array(
      '#type' => 'fieldset',
      '#title' => t('Verbose'),
      '#access' => user_access('monitoring verbose'),
    );
    $form['verbose'][$sensor_name] = array(
      '#type' => 'markup',
      '#markup' => '<pre>' . $verbose[$sensor_name] . '</pre>',
    );
  }

  $form['sensor_info'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sensor info'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['sensor_info'][$sensor_name] = array(
    '#type' => 'markup',
    '#markup' => '<pre>' . var_export($sensor_info, TRUE) . '</pre>',
  );

  return $form;
}

function monitoring_sensor_details_submit($form, &$form_state) {
  $result = monitoring_sensor_run($form_state['sensor_name'], TRUE);
  if (!empty($result)) {
    drupal_set_message(t('Sensor force run executed.'));
  }
  else {
    drupal_set_message(t('Error executing sensor force run.'), 'error');
  }
}

/**
 * Monitoring settings form.
 */
function monitoring_settings_form($form, &$form_state) {
  $form['monitoring_sensor_call_logging'] = array(
    '#type' => 'select',
    '#title' => t('Monitoring event logging'),
    '#description' => t('Control local logging of sensor call results.'),
    '#options' => array(
      'all' => t('Log all events'),
      'on_request' => t('Log only on request or on status change'),
      'none' => t('No logging')
    ),
    '#default_value' => variable_get('monitoring_sensor_call_logging', 'on_request'),
  );

  return system_settings_form($form);
}

/**
 * Ajax callback for threshold settings.
 */
function monitoring_sensor_thresholds_ajax($form, &$form_state) {
  $form_key = monitoring_sensor_settings_key($form_state['sensor']->getSensorName());
  return $form[$form_key]['thresholds']['intervals'];
}
