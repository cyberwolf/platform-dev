<?php

/**
 * @file
 * Code for the NextEuropa Core feature.
 */

/**
 * Implements hook_token_info_alter().
 */
function nexteuropa_core_token_info_alter(&$data) {

  $data['tokens']['term']['parents-uri'] = array(
    'name' => t("All parent terms, URI style"),
    'description' => t("Ancestors of the term, i.e. parent of all above hierarchy levels, separated by '/'."),
  );
}

/**
 * Implements hook_tokens().
 */
function nexteuropa_core_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();
  $separator = &drupal_static('nexteuropa_core_tokens_parents_uri_separator', '-parent-uri-separator-');

  if ($type == 'node' && isset($data['node'])) {
    $node = $data['node'];
    foreach ($tokens as $name => $original) {
      if (strstr($name, ':parents-uri') !== FALSE) {
        list($field_name, $delta,) = explode(':', $name);
        $field_name = str_replace('-', '_', $field_name);
        if (isset($node->{$field_name})) {
          $node_wrapper = entity_metadata_wrapper('node', $node);
          $terms = $node_wrapper->{$field_name}->value();
          if (isset($terms[$delta - 1])) {
            $term = $terms[$delta - 1];
            $parents = array();
            foreach (entity_metadata_taxonomy_term_get_properties($term, array(), 'parents_all') as $tid) {
              $parent_term = taxonomy_term_load($tid);
              $parents[] = $parent_term->name;
            }
            $replacements[$original] = implode($separator, array_reverse($parents));
          }
        }
      }
    }
  }

  if ($type == 'term' && isset($data['term'])) {
    $term = $data['term'];
    foreach ($tokens as $name => $original) {

      // Provide replacement value for [term:parents-uri] token.
      // It can only work paired-up with hook_pathauto_alias_alter()
      // since we have to provide an URI like replacement, containing '/'.
      if ($original == '[term:parents-uri]') {
        $parents = array();
        foreach (entity_metadata_taxonomy_term_get_properties($term, array(), 'parents_all') as $tid) {
          $parent_term = taxonomy_term_load($tid);
          $parents[] = $parent_term->name;
        }
        $replacements[$original] = implode($separator, array_reverse($parents));
      }
    }
  }

  return $replacements;
}

/**
 * Implements hook_pathauto_alias_alter().
 */
function nexteuropa_core_pathauto_alias_alter(&$alias, array &$context) {
  $separator = drupal_static('nexteuropa_core_tokens_parents_uri_separator');

  // Replace randomly generated separator with '/'.
  if (strstr($context['pattern'], '[term:parents-uri]')) {
    $alias = str_replace($separator, '/', $alias);
  }
  if (preg_match('/\[node:(.*)\:parents-uri\]/', $context['pattern'])) {
    $alias = str_replace($separator, '/', $alias);
  }
}
