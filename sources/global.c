#include "internal.h"

/** SDL_layer's internal global state. */
struct LYR_Global g_layer = { .initialized = false };

/** SDL_layer's Wayland registry listener. */
static const struct wl_registry_listener k_registry_listener;

/** Initialize SDL_layer. */
SDL_DECLSPEC bool SDLCALL LYR_Init(void) {
  SDL_PropertiesID props;

  // Avoid multiple initializations.
  if (g_layer.initialized) return true;

  // Obtain SDL's Wayland display connection.
  if ((props = SDL_GetGlobalProperties()) == 0) return false;
  if ((g_layer.display = SDL_GetPointerProperty(
    props, SDL_PROP_GLOBAL_VIDEO_WAYLAND_WL_DISPLAY_POINTER, NULL)) == NULL
  ) {
    SDL_SetError("SDL's Wayland display is uninitialized");
    goto return_error;
  }

  // Reset global pointers to unbound state.
  g_layer.compositor = NULL;
  g_layer.layer_shell = NULL;

  // Create a registry to enumerate globals.
  if ((g_layer.registry = wl_display_get_registry(g_layer.display)) == NULL) {
    SDL_SetError("Unable to create a new Wayland registry");
    goto return_error;
  }
  if (wl_registry_add_listener(g_layer.registry, &k_registry_listener, NULL) != 0) {
    SDL_SetError("Unable to attach Wayland registry listener");
    goto destroy_registry;
  }

  // Wait for enumeration and binding to complete.
  if (wl_display_roundtrip(g_layer.display) < 0) {
    SDL_SetError("Unable to enumerate Wayland globals");
    goto destroy_partial;
  }
  if (g_layer.compositor == NULL) {
    SDL_SetError("Missing Wayland compositor interface");
    goto destroy_partial;
  }
  if (g_layer.layer_shell == NULL) {
    SDL_SetError("Missing Wayland layer shell interface");
    goto destroy_partial;
  }
  if (g_layer.protocol_version < ZWLR_LAYER_SHELL_V1_DESTROY_SINCE_VERSION) {
    SDL_SetError("Unsupported layer shell protocol version");
    goto destroy_partial;
  }
  return (g_layer.initialized = true);

destroy_partial:
  if (g_layer.layer_shell != NULL) {
    zwlr_layer_shell_v1_destroy(g_layer.layer_shell);
  }
  if (g_layer.compositor != NULL) {
    wl_compositor_destroy(g_layer.compositor);
  }
destroy_registry:
  wl_registry_destroy(g_layer.registry);
return_error:
  return (g_layer.initialized = false);
}

/** Deinitialize SDL_layer. */
SDL_DECLSPEC void SDLCALL LYR_Quit(void) {
  if (!g_layer.initialized) return;
  zwlr_layer_shell_v1_destroy(g_layer.layer_shell);
  wl_compositor_destroy(g_layer.compositor);
  wl_registry_destroy(g_layer.registry);
  g_layer.initialized = false;
}

/** Handles the event of a Wayland global becoming available. */
static void LYR_OnGlobalCreate(
  SDL_UNUSED void               *opaque,
  struct wl_registry            *registry,
  uint32_t                       name,
  const char                    *interface,
  uint32_t                       version
) {
  // Only handle globals during library initialization.
  if (
    g_layer.initialized ||
    (g_layer.compositor != NULL && g_layer.layer_shell != NULL)
  ) return;

  // Try to match and bind an interface if it is not already bound.
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    if (g_layer.compositor == NULL) {
      g_layer.compositor = wl_registry_bind(
        registry, name, &wl_compositor_interface, version
      );
    }
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    if (g_layer.layer_shell == NULL) {
      g_layer.layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version
      );
      g_layer.protocol_version = version;
    }
  }
}

/** Ignores the event of a Wayland global becoming unavailable. */
static void LYR_OnGlobalRemove(
  SDL_UNUSED void               *opaque,
  SDL_UNUSED struct wl_registry *registry,
  SDL_UNUSED uint32_t            name
) {}

/** SDL_layer's global Wayland registry listener. */
static const struct wl_registry_listener k_registry_listener = {
  .global        = LYR_OnGlobalCreate,
  .global_remove = LYR_OnGlobalRemove,
};
