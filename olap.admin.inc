<?php

/**
 * @file
 */

/**
 * Menu callback.
 */
function olap_report() {
  $form_state = array(
    'method' => 'get',
    'rerender' => TRUE,
    'no_redirect' => TRUE,
    'always_process' => TRUE,
  );

  $form = drupal_build_form('olap_form', $form_state);
  return drupal_render($form);
}

/**
 * Form constructor.
 */
function olap_form($form, &$form_state) {
  if ($registers = register_get_all('RegisterTypeMovement')) {
    foreach ($registers as $register) {
      $schema = $register->getDbSchema('movement');

      foreach ($schema as $name => $table) {
        if (preg_match('/^r_(.+?)_movement$/', $name)) {
          $data[$name] = $table;
        }
      }
    }
  }

  $input = isset($form_state['input']) ? $form_state['input'] : array();
  $form['filter'] = _olap_report_filter($data);
  $form['data'] = _olap_data($data, $input);

  return $form;
}

function _olap_report_filter($data) {
  $form['type'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#title' => t('Select movement registry type'),
  );

  $form['fields'] = array(
    '#type' => 'select',
    '#title' => t('Select which fields should be displayed'),
    '#multiple' => TRUE,
  );

  $form['ipp'] = array(
    '#type' => 'select',
    '#options' => array_combine(array(5, 10, 50, 100), array(5, 10, 50, 100)),
    '#title' => t('Items per page'),
  );

  foreach ($data as $key => $table) {
    $form['type']['#options'][$key] = $key;

    foreach (array_keys($table['fields']) as $field) {
      $form['fields']['#options'][$field] = $field;
    }
  }

  $form['submit'] = array(
    '#name' => '',
    '#type' => 'submit',
    '#value' => t('Apply'),
  );

  $form = array('#type' => 'fieldset', $form);

  return $form;
}

function _olap_data($data, $input) {
  $header = array();
  $fields = isset($input['fields']) ? $input['fields'] : array('date', 'rtp', 'rid', 'rnm');

  foreach ($fields as $field) {
    $header[$field] = array('data' => t($field), 'field' => "m.$field");
  }

  $rows = db_select($input['type'], 'm')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('m', $fields)
    ->limit((empty($input['ipp']) ? 10 : $input['ipp']))
    ->orderByHeader($header)
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  _olad_field_date_format($rows, 'date');

  $form['data'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No content available.'),
  );

  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

function _olad_field_date_format(&$rows, $field) {
  if (isset($rows[0][$field])) {
    foreach ($rows as &$row) {
      $row[$field] = format_date($row[$field], 'short');
    }
  }
}
