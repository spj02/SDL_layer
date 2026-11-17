#include "internal.h"

/** Translates an SDL_layer anchor into its according Wayland anchor. */
static enum zwlr_layer_surface_v1_anchor LYR_GetWaylandAnchor(LYR_Anchor input) {
  enum zwlr_layer_surface_v1_anchor output;

  output = 0;
  if (input & LYR_ANCHOR_TOP) {
    output |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
  }
  if (input & LYR_ANCHOR_BOTTOM) {
    output |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
  }
  if (input & LYR_ANCHOR_LEFT) {
    output |= ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
  }
  if (input & LYR_ANCHOR_RIGHT) {
    output |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
  }
  return output;
}

/** Commits the given window creation properties to their surfaces. */
bool LYR_CommitSurfaceProperties(
  SDL_PropertiesID              props,
  struct wl_surface            *wl_surface,
  struct zwlr_layer_surface_v1 *layer_surface
) {
  bool ignore_pointer;
  struct wl_region *input_region;
  Sint64 width, height, keyboard_focus, exclusive_zone;
  enum zwlr_layer_surface_v1_anchor anchor, exclusive_edge;
  Sint64 margin_top, margin_right, margin_bottom, margin_left;

  // Extract relevant properties from the creation properties.
  width = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 0);
  height = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 0);
  anchor = LYR_GetWaylandAnchor(
    SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_ANCHOR_NUMBER, 0));
  margin_top = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_MARGIN_TOP_NUMBER, 0);
  margin_bottom = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_MARGIN_BOTTOM_NUMBER, 0);
  margin_left = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_MARGIN_LEFT_NUMBER, 0);
  margin_right = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_MARGIN_RIGHT_NUMBER, 0);
  keyboard_focus = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_KEYBOARD_FOCUS_NUMBER, 0);
  exclusive_zone = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_EXCLUSIVE_ZONE_NUMBER, 0);
  exclusive_edge = LYR_GetWaylandAnchor(
    SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_EXCLUSIVE_EDGE_NUMBER, 0));
  ignore_pointer =
    SDL_GetBooleanProperty(props, LYR_PROP_WINDOW_CREATE_IGNORE_POINTER_BOOLEAN, false);

  // Assign the surface's layout parameters.
  if (width != 0 || height != 0) {
    zwlr_layer_surface_v1_set_size(layer_surface, width, height);
  }
  if (anchor != 0) {
    zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
  }
  if (margin_top != 0 || margin_bottom != 0 || margin_left != 0 || margin_right != 0) {
    zwlr_layer_surface_v1_set_margin(
      layer_surface, margin_top, margin_right, margin_bottom, margin_left
    );
  }

  // Assign the according keyboard interactivity mode.
  switch (keyboard_focus) {
    case LYR_FOCUS_NONE:
      break;
    case LYR_FOCUS_EXCLUSIVE:
      zwlr_layer_surface_v1_set_keyboard_interactivity(
        layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE
      );
      break;
    case LYR_FOCUS_ON_DEMAND:
      if (g_layer.protocol_version < 
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND_SINCE_VERSION)
      {
        SDL_SetError("The compositor does not support on-demand keyboard interactivity");
        return false;
      }
      zwlr_layer_surface_v1_set_keyboard_interactivity(
        layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND
      );
      break;
    default:
      SDL_SetError("The keyboard focus mode is unknown");
      return false;
  }

  // Validate and assign the surface's exclusive zone and edge.
  if (exclusive_zone != 0) {
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, exclusive_zone);
  }
  if (exclusive_edge != 0) {
    if (g_layer.protocol_version < ZWLR_LAYER_SURFACE_V1_SET_EXCLUSIVE_EDGE_SINCE_VERSION) {
      SDL_SetError("The compositor does not support exclusive edges");
      return false;
    }
    switch (exclusive_edge) {
      case ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP:
      case ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM:
      case ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT:
      case ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT:
        if ((anchor & exclusive_edge) == 0) {
          SDL_SetError("The exclusive edge is not anchored");
          return false;
        }
        break;
      default:
        SDL_SetError("The exclusive edge is unknown");
        return false;
    }
    zwlr_layer_surface_v1_set_exclusive_edge(layer_surface, exclusive_edge);
  }

  // Create and assign an empty input region when pointer input should be ignored.
  if (ignore_pointer) {
    if ((input_region = wl_compositor_create_region(g_layer.compositor)) == NULL) {
      SDL_SetError("Unable to create input region");
      return false;
    }
    wl_surface_set_input_region(wl_surface, input_region);
  } else {
    input_region = NULL;
  }

  // Commit the surface changes and release temporary objects.
  wl_surface_commit(wl_surface);
  if (input_region != NULL) wl_region_destroy(input_region);
  return true;
}

/** Obtains the desired Wayland output from the given window creation properties. */
bool LYR_GetWaylandOutput(SDL_PropertiesID props, struct wl_output **r_output) {
  SDL_DisplayID display_id;
  SDL_PropertiesID display_props;
  struct wl_output *display_output;

  if ((display_id = SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_DISPLAY_NUMBER, 0)) == 0) {
    *r_output = NULL;
    return true;
  }
  if ((display_props = SDL_GetDisplayProperties(display_id)) == 0) {
    return false;
  }
  display_output = SDL_GetPointerProperty(
    display_props, SDL_PROP_DISPLAY_WAYLAND_WL_OUTPUT_POINTER, NULL
  );
  if (display_output == NULL) {
    SDL_SetError("Display has no Wayland output");
    return false;
  }
  *r_output = display_output;
  return true;
}

/** Obtains the desired Wayland layer from the given window creation properties. */
bool LYR_GetWaylandLayer(SDL_PropertiesID props, enum zwlr_layer_shell_v1_layer *r_layer) {
  switch (SDL_GetNumberProperty(props, LYR_PROP_WINDOW_CREATE_LAYER_NUMBER, LYR_LAYER_INVALID)) {
    case LYR_LAYER_INVALID:
      SDL_SetError("The desired layer is not set");
      return false;
    case LYR_LAYER_BACKGROUND:
      *r_layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
      return true;
    case LYR_LAYER_BOTTOM:
      *r_layer = ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;
      return true;
    case LYR_LAYER_TOP:
      *r_layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
      return true;
    case LYR_LAYER_OVERLAY:
      *r_layer = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY;
      return true;
    default:
      SDL_SetError("The desired layer is unknown");
      return false;
  }
}
