#include "internal.h"

/** Internal property for initial surface configuration. */
#define LYR_PROP_WINDOW_CONFIGURED_BOOLEAN "LYR.window.configured"

/** SDL_layer's Wayland layer surface listener. */
static const struct zwlr_layer_surface_v1_listener k_layer_surface_listener;

/** Destroys a Wayland layer surface stored in a pointer property. */
static void LYR_DestroyLayerSurface(SDL_UNUSED void *userdata, void *value) {
  zwlr_layer_surface_v1_destroy(value);
}

/** Create a layer window with the specified properties. */
SDL_DECLSPEC SDL_Window *SDLCALL LYR_CreateWindow(SDL_PropertiesID props) {
  SDL_Window *window;
  const char *namespace;
  struct wl_output *wl_output;
  struct wl_surface *wl_surface;
  SDL_PropertiesID window_props;
  enum zwlr_layer_shell_v1_layer layer;
  struct zwlr_layer_surface_v1 *layer_surface;

  // Require the internal global state to be initialized.
  if (!g_layer.initialized) {
    SDL_SetError("SDL_layer is uninitialized");
    goto return_error;
  }

  // Extract relevant properties from the creation properties.
  if (!LYR_GetWaylandLayer(props, &layer)) {
    goto return_error;
  }
  if (!LYR_GetWaylandOutput(props, &wl_output)) {
    goto return_error;
  }
  namespace = SDL_GetStringProperty(
    props, LYR_PROP_WINDOW_CREATE_NAMESPACE_STRING, "SDL_layer"
  );

  // Create an SDL window without a surface role.
  if (!SDL_SetBooleanProperty(
    props, SDL_PROP_WINDOW_CREATE_WAYLAND_SURFACE_ROLE_CUSTOM_BOOLEAN, true
  )) {
    goto return_error;
  }
  if ((window = SDL_CreateWindowWithProperties(props)) == NULL) {
    goto return_error;
  }

  // Obtain the window's Wayland surface pointer.
  if ((window_props = SDL_GetWindowProperties(window)) == 0) {
    goto destroy_window;
  }
  wl_surface = SDL_GetPointerProperty(
    window_props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL
  );
  if (wl_surface == NULL) {
    SDL_SetError("Window has no Wayland surface");
    goto destroy_window;
  }

  // Create a layer surface owned by the window's property group.
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
    g_layer.layer_shell, wl_surface, wl_output, layer, namespace
  );
  if (layer_surface == NULL) {
    SDL_SetError("Unable to create Wayland layer surface");
    goto destroy_window;
  }
  if (!SDL_SetPointerPropertyWithCleanup(
    window_props,
    LYR_PROP_WINDOW_LAYER_SURFACE_POINTER,
    layer_surface,
    LYR_DestroyLayerSurface,
    NULL
  )) {
    zwlr_layer_surface_v1_destroy(layer_surface);
    goto destroy_window;
  }

  // Configure the layer surface using the creation properties.
  if (zwlr_layer_surface_v1_add_listener(layer_surface, &k_layer_surface_listener, window) != 0) {
    SDL_SetError("Unable to attach layer surface listener");
    goto destroy_window;
  }
  if (!LYR_CommitSurfaceProperties(props, wl_surface, layer_surface)) {
    goto destroy_window;
  }
  if (wl_display_roundtrip(g_layer.display) < 0) {
    SDL_SetError("Unable to await layer surface configuration");
    goto destroy_window;
  }

  // Expect the layer surface to be successfully configured.
  if (!SDL_GetBooleanProperty(window_props, LYR_PROP_WINDOW_CONFIGURED_BOOLEAN, false)) {
    SDL_SetError("The layer surface did not receive a configuration event");
    goto destroy_window;
  }
  return window;

destroy_window:
  SDL_DestroyWindow(window);
return_error:
  return NULL;
}

/** Handles the event of a layer surface offering a configuration. */
static void LYR_OnConfigure(
  void                                    *opaque,
  struct zwlr_layer_surface_v1            *layer_surface,
  uint32_t                                 serial,
  uint32_t                                 width,
  uint32_t                                 height
) {
  SDL_Window *window;
  SDL_PropertiesID props;

  // Update the window's size and acknowledge the configuration.
  window = opaque;
  if ((props = SDL_GetWindowProperties(window)) == 0) return;
  SDL_SetWindowSize(window, width, height);
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

  // Mark the window as configured so its creation can be completed.
  SDL_SetBooleanProperty(props, LYR_PROP_WINDOW_CONFIGURED_BOOLEAN, true);
}

/** Handles the event of a layer surface being closed. */
static void LYR_OnClosed(
  void                                    *opaque,
  SDL_UNUSED struct zwlr_layer_surface_v1 *layer_surface
) {
  SDL_Event event;
  SDL_Window **windows, *window;
  int window_index, window_count, toplevel_count;

  // Push a close request for the surface's window.
  window = opaque;
  SDL_zero(event);
  event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
  if ((event.window.windowID = SDL_GetWindowID(window)) == 0) return;
  SDL_PushEvent(&event);

  // Emulate SDL's quit-on-last-close behavior if not disabled.
  if (!SDL_GetHintBoolean(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, true)) {
    // Count the number of visible toplevel windows.
    toplevel_count = 0;
    if ((windows = SDL_GetWindows(&window_count)) != NULL) {
      for (window_index = 0; window_index < window_count; window_index++) {
        window = windows[window_index];
        if (
          SDL_GetWindowParent(window) == NULL &&
          (SDL_GetWindowFlags(window) & SDL_WINDOW_HIDDEN) == 0
        ) {
          toplevel_count++;
        }
      }
      SDL_free(windows);
    }

    // Push a quit event if this window is the last one.
    if (toplevel_count <= 1) {
      SDL_zero(event);
      event.type = SDL_EVENT_QUIT;
      SDL_PushEvent(&event);
    }
  }
}

/** SDL_layer's Wayland layer surface listener. */
static const struct zwlr_layer_surface_v1_listener k_layer_surface_listener = {
  .configure = LYR_OnConfigure,
  .closed    = LYR_OnClosed
};
