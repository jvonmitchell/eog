#ifndef __EOG_RELOAD_PLUGIN_H__
#define __EOG_RELOAD_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

#include <eog-window.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define EOG_TYPE_RELOAD_PLUGIN		(eog_reload_plugin_get_type ())
#define EOG_RELOAD_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), EOG_TYPE_RELOAD_PLUGIN, EogReloadPlugin))
#define EOG_RELOAD_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k),     EOG_TYPE_RELOAD_PLUGIN, EogReloadPluginClass))
#define EOG_IS_RELOAD_PLUGIN(o)	        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EOG_TYPE_RELOAD_PLUGIN))
#define EOG_IS_RELOAD_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k),    EOG_TYPE_RELOAD_PLUGIN))
#define EOG_RELOAD_PLUGIN_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o),  EOG_TYPE_RELOAD_PLUGIN, EogReloadPluginClass))

/* Private structure type */
typedef struct _EogReloadPluginPrivate	EogReloadPluginPrivate;

/*
 * Main object structure
 */
typedef struct _EogReloadPlugin		EogReloadPlugin;

struct _EogReloadPlugin
{
	PeasExtensionBase parent_instance;

	EogWindow *window;
	GtkActionGroup *ui_action_group;
	guint ui_id;
};

/*
 * Class definition
 */
typedef struct _EogReloadPluginClass	EogReloadPluginClass;

struct _EogReloadPluginClass
{
	PeasExtensionBaseClass parent_class;
};

/*
 * Public methods
 */
GType	eog_reload_plugin_get_type		(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif /* __EOG_RELOAD_PLUGIN_H__ */
