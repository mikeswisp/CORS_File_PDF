<?php
/**
 * @file that create the settings and formatting options for the PDF field
 * Much of this functionality is based off of the PDF module
 *
 * PDF Module
 * @link https://www.drupal.org/project/pdf
 *
 * Github Repo for php-opencloud
 * @link https://github.com/rackspace/php-opencloud/tree/working/samples
 */

/**
 * Implements hook_field_formatter_info().
 * Formatters handle the display of field values
 *
 * @return array
 *  An array describing the formatter types implemented by the module
 */
function cors_file_pdf_field_formatter_info() {
  return array(
    'pdf_rackspace' => array(
      'label' => t('Rackspace PDF: Using Default viewer of PDF.js'),
      'description' => 'Use the default viewer like http://mozilla.github.io/pdf.js/web/viewer.html.',
      'field types' => array('file'),
      'settings'=>array(
        'keep_pdfjs' => TRUE,
        'width' => '100px',
        'height' => '600px',
      ),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form()
 *
 * @param $field
 *  The field structure being configured.
 *
 * @param $instance
 *  The instance structure being configured.
 *
 * @param $view_mode
 *  The view mode being configured.
 *
 * @param $form
 *  The (entire) configuration form array, which will usually have no use here.
 *
 * @param $form_state
 *  The form state of the (entire) configuration form.
 *
 * @return array
 *  The form elements for the formatter settings.
 */
function cors_file_pdf_field_formatter_settings_form(
  $field, $instance, $view_mode, $form, &$form_state) {

  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  if ($display['type'] == 'pdf_rackspace') {

    $element['keep_pdfjs'] = array(
      '#type' => 'checkbox',
      '#title' => t('Always use pdf.js'),
      '#default_value' => $settings['keep_pdfjs'],
      '#description' => t("Use pdf.js even the browser has Adobe Reader Plugin, WebKit PDF Reader for Safari or the PDF Reader for Chrome (Chrome's default alternative to the Adobe Reader Plugin) installed."),
    );

    $element['width'] = array(
      '#type' => 'textfield',
      '#title' => 'Width',
      '#default_value' => $settings['width'],
      '#description' => t('Width of the viewer. Ex: 250px or 100%'),
    );

    $element['height'] = array(
      '#type' => 'textfield',
      '#title' => 'Height',
      '#default_value' => $settings['height'],
      '#description' => t('Height of the viewer. Ex: 250px or 100%'),
    );

  }

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 *
 * @param $field
 *  The field structure.
 *
 * @param $instance
 *  The instance structure.
 *
 * @param $view_mode
 *  The view mode for which a settings summary is requested.
 *
 * @return string
 *  A string containing a short summary of the formatter settings
 */
function cors_file_pdf_field_formatter_settings_summary($field, $instance, $view_mode) {

  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  if ($display['type'] == 'pdf_rackspace') {
    return t('Use pdf.js even users have PDF reader plugin: @keep_pdfjs',
              array('@keep_pdfjs' => $settings['keep_pdfjs'] ? t('Yes') : t('No'))) . '<br>' . t('Widht: @width , Height: @height',
              array('@width' => $settings['width'] , '@height' => $settings['height'])
           );
  }

  return '';
}

/**
 * Implements hook_field_formatter_view().
 *
 * @param $entity_type
 *   The type of $entity.
 *
 * @param $entity
 *   The entity being displayed.
 *
 * @param $field
 *   The field structure.
 *
 * @param $instance
 *   The field instance.
 *
 * @param $langcode
 *   The language associated with $items.
 *
 * @param $items
 *   Array of values for this field.
 *
 * @return array
 *  Associated array that defines what the formatting is supposed to be
 */
function cors_file_pdf_field_formatter_view(
  $entity_type, $entity, $field,
  $instance, $langcode, $items, $display) {

  $element = array();
  $settings = $display['settings'];

  switch ($display['type']) {
    case 'pdf_rackspace':

      foreach ($items as $delta => $item) {
        if ( strpos($item['filemime'], 'pdf' ) ) {
          $element[$delta] = array(
            '#theme' => 'rackspace_cors_file_formatter',
            '#file' => (object)$item,
            '#keep_pdfjs' => $settings['keep_pdfjs'],
            '#width'  => $settings['width'],
            '#height'  => $settings['height'],
          );
        }
      }
      break;
  }
  return $element;
}

/**
 * The field theme defined by hook_theme()
 *
 * @param $variables
 *  The callback data associated with this field
 *
 * @return array
 *  The render array for the field to be displayed
 */
function theme_rackspace_cors_file_formatter($variables) {

  $library = libraries_load('pdf.js');

  if ($library['loaded'] == FALSE) {
    drupal_set_message($library['error message'], 'error');
    return 'Please download and install ' . l( $library['name'], $library['download url'] ) . '!';
  }

  $uri = _cors_file_pdf_get_file($variables['file']->uri);
  if ($uri != NULL) {
    $file_url = file_create_url($uri);
  } else {
    $content = array(
      '#type' => 'html_tag',
      '#tag' => 'p',
      '#value' => t('There was an error downloading your file'),
    );
    return render($content);
  }


  $module_path = drupal_get_path('module', 'cors_file_pdf');
  $library_path = libraries_get_path('pdf.js');
  $iframe_src = file_create_url($library_path . '/web/viewer.html') . '?file=' . rawurlencode($file_url);
  $force_pdfjs = $variables['keep_pdfjs'];


  $content = array(
    '#type' => 'html_tag',
    '#tag' => 'iframe',
    '#value' => $file_url,
    '#attributes' => array(
      'class' => array('cors_file_pdf'),
      'webkitallowfullscreen' => '',
      'mozallowfullscreen' => '',
      'allowfullscreen' => '',
      'frameborder' => 'no',
      'width' => $variables['width'],
      'height' => $variables['height'],
      'src' => $iframe_src,
      'data-src' => $file_url,
    ),
  );
  if ($force_pdfjs != TRUE) {
    drupal_add_js($module_path . '/js/acrobat_detection.js');
    drupal_add_js($module_path . '/js/default.js');
  }

  return render($content);
}

/**
 * Utility function that securely downloads a file from Rackspace
 * and writes the public files directory for 3 hours
 *
 * @param $uri
 *  The uri for the file that needs to be downloaded
 *
 * @return string
 *  The path to the local file
 */
function _cors_file_pdf_get_file($uri) {

  $uri_exploded = explode('://', $uri);
  $local_uri = '';
  // check to make sure the file does not already exist
  if (!empty($uri_exploded[1])) {
    $base_uri_array = explode('/', $uri_exploded[1]);
    $length = count($base_uri_array);
    if ($length > 0) {
      $local_uri = 'public://' . $base_uri_array[($length - 1)];
    }
  }

  if (file_exists($local_uri)) {
    // Return local reference
    return $local_uri;
  }

  // Reference the stream wrapper from the cors file module
  $wrapper = new RackspaceStreamWrapper();
  try {
    $info = libraries_load('php-opencloud');
    if ($info['loaded']) {
      // Get the client
      $client = $wrapper->getClient();
      // Get the object store
      $object_store_service = $client->objectStoreService(
        'cloudFiles',
        variable_get('rackspace_region'),
        'publicURL'
      );

      // Set the container
      $container = $object_store_service->getContainer(
        variable_get('rackspace_container')
      );

      if (strpos($uri, '://') != -1) {
        if (!empty($uri_exploded[1])) {
          $uri = $uri_exploded[1];
        } else {
          $uri = '';
        }
      }
      // Taken from php-opencloud sample gihub repo
      // https://github.com/rackspace/php-opencloud/tree/working/samples
      $file_object = $container->getObject($uri);
      $object_content = $file_object->getContent();
      $object_content->rewind();
      $stream = $object_content->getStream();

      $base_uri_array = explode('/', $uri);
      if (isset($base_uri_array[1])) {
        $uri = $base_uri_array[1];
      }
      $file = file_unmanaged_save_data($stream, 'public://' .$uri);
      return $file;

    } else {
      return NULL;
    }
  } catch(Exception $e) {
    _cors_file_pdf_write_watchdog($e->getMessage());
    return NULL;
  }
}

/**
 * Utility function to write a watchdog record
 *
 * @param $message
 *  The message we are writing to watchdog
 */
function _cors_file_pdf_write_watchdog($message) {
  watchdog(
    "cors_file_pdf",
    $message,
    array(),
    WATCHDOG_DEBUG
  );
}
