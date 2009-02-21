/* $Id$ */
(function($) {

Drupal.adminMenu = Drupal.adminMenu || {};

/**
 * Core behavior for Administration menu.
 *
 * This tests whether there is an administration menu is in the output and
 * executes all registered behaviors.
 */
Drupal.behaviors.adminMenu = function (context) {
  var $adminMenu = $('#admin-menu');
  if ($adminMenu.size()) {
    $.each(Drupal.adminMenu, function() {
      this(context, $adminMenu);
    });
  }
};

/**
 * Collapse fieldsets on Modules page.
 *
 * For why multiple selectors see #111719.
 */
Drupal.behaviors.adminMenuCollapseModules = function (context) {
  if (Drupal.settings.admin_menu && Drupal.settings.admin_menu.tweak_modules) {
    $('#system-modules fieldset:not(.collapsed), #system-modules-1 fieldset:not(.collapsed)', context).addClass('collapsed');
  }
};

/**
 * Apply 'margin-top'; directly applying marginTop does not work in IE.
 */
Drupal.adminMenu.marginTop = function (context, $adminMenu) {
  if (Drupal.settings.admin_menu && Drupal.settings.admin_menu.margin_top) {
    $('body', context).addClass('admin-menu');
  }
};

/**
 * Apply 'position: fixed'.
 */
Drupal.adminMenu.positionFixed = function (context, $adminMenu) {
  if (Drupal.settings.admin_menu && Drupal.settings.admin_menu.position_fixed) {
    $adminMenu.css('position', 'fixed');
  }
};

/**
 * Move page tabs into administration menu.
 */
Drupal.adminMenu.pageTabs = function (context, $adminMenu) {
  if (Drupal.settings.admin_menu && Drupal.settings.admin_menu.tweak_tabs) {
    $('ul.tabs.primary li', context).each(function() {
      $(this).addClass('admin-menu-tab').appendTo('#admin-menu > ul');
    });
    $('ul.tabs.secondary').appendTo('#admin-menu > ul > li.admin-menu-tab.active');
  }
};

/**
 * Apply JavaScript-based hovering behaviors.
 *
 * @todo This has to run last.  If another script registers additional behaviors
 *   it will not run last.
 */
Drupal.adminMenu.hover = function (context, $adminMenu) {
  // Hover emulation for IE 6.
  if ($.browser.msie && parseInt(jQuery.browser.version) == 6) {
    $('li', $adminMenu).hover(function() {
      $(this).addClass('iehover');
    }, function() {
      $(this).removeClass('iehover');
    });
  }

  // Delayed mouseout.
  $('li', $adminMenu).hover(function() {
    // Stop the timer.
    clearTimeout(this.sfTimer);
    // Display child lists.
    $('> ul', this).css({left: 'auto', display: 'block'})
      // Immediately hide nephew lists.
      .parent().siblings('li').children('ul').css({left: '-999em', display: 'none'});
  }, function() {
    // Start the timer.
    var uls = $('> ul', this);
    this.sfTimer = setTimeout(function() {
      uls.css({left: '-999em', display: 'none'});
    }, 400);
  });
};

})(jQuery);
