#ifndef SDL_layer_internal_h_
#define SDL_layer_internal_h_

#include <SDL3_layer.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <layer-shell-client-protocol.h>

/** SDL_layer's internal global state. */
extern struct LYR_Global {
  struct wl_display          *display;
  struct wl_registry         *registry;
  struct wl_compositor       *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  uint32_t                    protocol_version;
  bool                        initialized;
} g_layer;

/** Commits the given window creation properties to their surfaces. */
bool LYR_CommitSurfaceProperties(
  SDL_PropertiesID              props,
  struct wl_surface            *wl_surface,
  struct zwlr_layer_surface_v1 *layer_surface
);

/** Obtains the desired Wayland output from the given window creation properties. */
bool LYR_GetWaylandOutput(SDL_PropertiesID props, struct wl_output **r_output);

/** Obtains the desired Wayland layer from the given window creation properties. */
bool LYR_GetWaylandLayer(SDL_PropertiesID props, enum zwlr_layer_shell_v1_layer *r_layer);

#endif // SDL_layer_internal_h_
