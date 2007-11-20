<?php
// $Id$

/**
 * Retrieves, sclices and returns the menu tree below /admin.
 */
function admin_menu_get_menu() {
  global $_menu;
  $admin_items = array();

  // get path id of /q=admin, which we suppose to be the root of admin menu
  $mid_admin = $_menu['path index']['admin'];
  
  // Temporary access permissions fix for root menu items until #126621 is committed to core.
  foreach ($_menu['items'][$mid_admin]['children'] as $key => $parent) {
    if ($_menu['items'][$parent]['access'] != false || !isset($_menu['items'][$parent]['children'])) {
      continue;
    }
    $is_accessible = false;
    foreach ($_menu['items'][$parent]['children'] as $key => $child) {
      if ($_menu['items'][$child]['access'] == true) {
        $is_accessible = true;
      }
    }
    if ($is_accessible) {
      $_menu['items'][$parent]['access'] = true;
    }
  }
  
  // slice admin menu items into a new menu tree
  $admin_items[$mid_admin] = $_menu['items'][$mid_admin];
  _admin_menu_get_children($admin_items, $admin_items[$mid_admin]);
  
  // adjust some specific menu items for better user experience
  admin_menu_adjust_items($admin_items);
  
  // remove administer menu item from parent's children list
  $parent_id = $_menu['items'][$mid_admin]['pid'];
  $child_key = array_search($mid_admin, $_menu['visible'][$parent_id]['children']);
  unset($_menu['visible'][$parent_id]['children'][$child_key]);
  
  return theme('admin_menu_tree', $admin_items, $mid_admin);
}

function _admin_menu_get_children(&$admin_items, &$item) {
  global $_menu;
  
  if (isset($item['children'])) {
    foreach ($item['children'] as $child) {
      // check access permissions
      if (!variable_get('admin_menu_show_all', 0)) {
        $item_is_accessible = !isset($_menu['items'][$child]['access']) || (isset($_menu['items'][$child]['access']) && $_menu['items'][$child]['access']);
      }
      else {
        $item_is_accessible = true;
      }
      // check type
      $item_is_visible = $_menu['items'][$child]['type'] & (MENU_VISIBLE_IN_TREE | MENU_IS_LOCAL_TASK);

      // create the child item if it is accessible and visible
      // Additional if condition to hide items linking to parent:
      //   && !($_menu['items'][$child]['type'] & MENU_LINKS_TO_PARENT)
      if ($item_is_accessible && $item_is_visible) {
        $admin_items[$child] = $_menu['items'][$child];

        // recurse to child menu items
        if (isset($_menu['items'][$child]['children'])) {
          _admin_menu_get_children($admin_items, $admin_items[$child]);
        }

        // remove this child item if it is visible in navigation
        unset($_menu['visible'][$child]);
      }
      else {
        // remove child menu item from parent's children list
        $parent_id = $_menu['items'][$child]['pid'];
        if (isset($_menu['items'][$parent_id]['children']) && is_array($_menu['items'][$parent_id]['children'])) {
          $child_key = array_search($child, $_menu['items'][$parent_id]['children']);
          if ($child_key !== false) {
            unset($admin_items[$parent_id]['children'][$child_key]);
          }
        }
      }
    }

    // sort remaining items
    usort($item['children'], '_menu_sort');
  }
}

function admin_menu_adjust_items(&$admin_items) {
  global $_menu;
  
  // move 'By module' into Site configuration or remove it
  if (user_access('administer site configuration')) {
    admin_menu_move_item($admin_items, 'admin/by-module', 'admin/settings');
  }
  else {
    admin_menu_remove_item($admin_items, 'admin/by-module');
  }
  
  // remove 'By task' menu item
  admin_menu_remove_item($admin_items, 'admin/by-task');

  // add logout item
  $mid_logout = $_menu['path index']['logout'];
  $admin_items[$mid_logout] = $_menu['items'][$mid_logout];
  $admin_items[$mid_logout]['class'] = 'admin_menu-action';
  $mid_admin = $_menu['path index']['admin'];
  $admin_items[$mid_logout]['pid'] = $mid_admin;
  array_unshift($admin_items[$mid_admin]['children'], $mid_logout);
  
  // add create content_type items to Content management > Content
  $mid_node_add = $_menu['path index']['node/add'];
  $mid_content = $_menu['path index']['admin/content/node'];
  admin_menu_copy_items($admin_items, $mid_node_add, $mid_content, t('Add !title'));
}

/**
 * Recursively copy menu items from a source parent menu item to a target menu
 * item.
 * 
 * @param admin_items
 *   The sliced administration menu array.
 * @param source_pid
 *   A source parent menu item id from which children shall be copied.
 * @param target_pid
 *   A target parent menu item id.
 * @param title
 *   An optional string containing the token !title, that has already been
 *   passed through t().
 * @param tree
 *   Whether to rebuild hierarchy from source parent menu item. Defaults to
 *   true.
 */
function admin_menu_copy_items(&$admin_items, $source_pid, $target_pid, $title = NULL, $tree = TRUE) {
  global $_menu;
  
  if (isset($admin_items[$target_pid]) && isset($_menu['items'][$source_pid]['children'])) {
    foreach ($_menu['items'][$source_pid]['children'] as $mid) {
      $admin_items[$mid] = $_menu['items'][$mid];
      if (isset($title)) {
        $admin_items[$mid]['title'] = check_plain(strtr($title, array('!title' => $admin_items[$mid]['title'])));
      }
      $admin_items[$mid]['pid'] = $target_pid;
      // Only add child to target menu item if it does not already exist.
      if (!in_array($mid, $admin_items[$target_pid]['children'])) {
        $admin_items[$target_pid]['children'][] = $mid;
      }
      
      // Recurse into children.
      if (isset($_menu['items'][$mid]['children']) && count($_menu['items'][$mid]['children'])) {
        if ($tree) {
          admin_menu_copy_items($admin_items, $mid, $mid, $title);
        }
        else {
          admin_menu_copy_items($admin_items, $mid, $target_pid, $title, FALSE);
          unset($admin_items[$mid]['children']);
          // Note:
          // Uncomment following lines to also optionally remove unnecessary
          // parent items.
          // unset($admin_items[$target_pid]['children'][array_search($mid, $admin_items[$target_pid]['children'])]);
          // unset($admin_items[$mid]);
        }
      }
    }
    // sort childs
    usort($admin_items[$target_pid]['children'], '_menu_sort');
  }
}

/**
 * Moves the child pointer for a menu item to a new parent.
 * 
 * @param admin_items
 *      The sliced administration menu.
 * @param path
 *      The menu path of the item to move.
 * @param parent_path
 *      The menu path of the new parent item.
 */
function admin_menu_move_item(&$admin_items, $path, $parent_path) {
  global $_menu;
  
  $mid     = $_menu['path index'][$path];
  $new_pid = $_menu['path index'][$parent_path];
  if (isset($admin_items[$mid]) && isset($admin_items[$new_pid])) {
    // remove current child pointer
    admin_menu_remove_item($admin_items, $path);
    
    // insert new child pointer
    $admin_items[$mid]['pid'] = $new_pid;
    array_unshift($admin_items[$new_pid]['children'], $mid);
    
    return true;
  }
  else {
    return false;
  }
}

/**
 * Removes the child pointer for a menu item.
 * 
 * @param admin_items
 *      The sliced administration menu.
 * @param path
 *      The menu path to remove.
 */
function admin_menu_remove_item(&$admin_items, $path) {
  global $_menu;
  
  $mid = $_menu['path index'][$path];
  if (isset($admin_items[$mid])) {
    $pid = $admin_items[$mid]['pid'];
    $child_key = array_search($mid, $admin_items[$pid]['children']);
    if ($child_key !== false) {
      unset($admin_items[$pid]['children'][$child_key]);
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}

/**
 * Returns the rendered link to a menu item.
 *
 * @param $mid
 *   The menu item id to render.
 */
function admin_menu_item_link(&$admin_items, $mid) {
  $item = $admin_items[$mid];
  $link_item = $item;

  while ($link_item['type'] & MENU_LINKS_TO_PARENT) {
    $link_item = menu_get_item($link_item['pid']);
  }

  // display menu item ids if enabled (devel)
  if (variable_get('admin_menu_show_item_ids', 0)) {
    $item['title'] = $item['title'] . ' (' . $mid . ')';
  }
  
  // Remove menu item tooltips due to mouseover clashes.
  unset($item['description']);
  
  return theme('menu_item_link', $item, $link_item);
}

/**
 * Generate the HTML for a menu tree.
 *
 * @param $pid
 *   The parent id of the menu.
 *
 * @ingroup themeable
 */
function theme_admin_menu_tree(&$menu, $pid = 1) {
  $output = '';
  
  if (isset($menu[$pid]) && $menu[$pid]['children']) {
    foreach ($menu[$pid]['children'] as $mid) {
      $type = isset($menu[$mid]['type']) ? $menu[$mid]['type'] : null;
      $children = isset($menu[$mid]['children']) ? $menu[$mid]['children'] : null;
      $output .= theme('admin_menu_item', $menu, $mid, theme('admin_menu_tree', $menu, $mid), count($children) == 0);
    }
  }
  return $output ? "<ul>" . $output . "</ul>" : '';
}

function theme_admin_menu_item(&$admin_items, $mid, $children = '', $leaf = true) {
  $class = array();
  if (!$leaf) {
    $class[] = 'expandable';
  }
  if (isset($admin_items[$mid]['class'])) {
    $class[] = $admin_items[$mid]['class'];
  }
  $output  = '<li'. (!empty($class) ? ' class="'. implode(' ', $class) .'"' : '') . '>';
  $output .= admin_menu_item_link($admin_items, $mid) . $children . '</li>';
  return $output;
}
