(function($) {

Drupal.admin = Drupal.admin || {};
Drupal.admin.behaviors = Drupal.admin.behaviors || {};

/**
 * Apply margin to page, taking into account the shortcuts.
 *
 * @see Drupal.behaviors.adminMenuMarginTop()
 */
Drupal.behaviors.adminMenuMarginTopShortcuts = {
  attach: function (context, settings) {
    if (!settings.admin_menu.suppress && settings.admin_menu.toolbar.margin_top_shortcuts) {
      $('body:not(.admin-menu-with-shortcuts)', context).addClass('admin-menu-with-shortcuts');
    }
  }
};

/**
 * @ingroup admin_behaviors
 * @{
 */

/**
 * Apply active trail highlighting based on current path.
 *
 * @todo Not limited to toolbar; move into core?
 */
Drupal.admin.behaviors.toolbarActiveTrail = function (context, settings, $adminMenu) {
  if (settings.admin_menu.toolbar && settings.admin_menu.toolbar.activeTrail) {
    $adminMenu.find('> div > ul > li > a[href="' + settings.admin_menu.toolbar.activeTrail + '"]').addClass('active-trail');
  }
};

/**
 * Toggle the shortcut drawer.
 */
Drupal.admin.behaviors.toggleShortcutDrawer = function (context, settings, $adminMenu) {
  var $body = $('body', context);
  var $shortcuts = $('li.admin-menu-shortcuts', $adminMenu);
  var $toggle = $('.toggle', $shortcuts);
  $toggle.toggle(
    function () {
      $shortcuts.addClass('admin-menu-shortcuts-inactive').removeClass('admin-menu-shortcuts-active');
      $body.removeClass('admin-menu-with-shortcuts');
      return false;
    },
    function () {
      $shortcuts.addClass('admin-menu-shortcuts-active').removeClass('admin-menu-shortcuts-inactive');
      $body.addClass('admin-menu-with-shortcuts');
      return false;
    }
  );
};

/**
 * @} End of "defgroup admin_behaviors".
 */

})(jQuery);
