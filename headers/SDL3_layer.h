#ifndef SDL_layer_h_
#define SDL_layer_h_

#include <SDL3/SDL.h>
#include <SDL3/SDL_begin_code.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Possible layers to place a window on.
 *
 * \since This enum is available since SDL_layer 1.0.0.
 *
 * \sa LYR_CreateWindow
 * \sa LYR_PROP_WINDOW_CREATE_LAYER_NUMBER
 */
typedef enum LYR_Layer {
  LYR_LAYER_INVALID    = 0, /**< The layer is unspecified. */
  LYR_LAYER_BACKGROUND = 1, /**< Place the window on the background layer. */
  LYR_LAYER_BOTTOM     = 2, /**< Place the window on the bottom layer. */
  LYR_LAYER_TOP        = 3, /**< Place the window on the top layer. */
  LYR_LAYER_OVERLAY    = 4  /**< Place the window on the overlay layer. */
} LYR_Layer;

/**
 * Bit set of anchor rectangle edges.
 *
 * \since This enum is available since SDL_layer 1.0.0.
 *
 * \sa LYR_CreateWindow
 * \sa LYR_PROP_WINDOW_CREATE_ANCHOR_NUMBER
 */
typedef enum LYR_Anchor {
  LYR_ANCHOR_TOP       = 1, /**< The anchor rectangle's top edge. */
  LYR_ANCHOR_BOTTOM    = 2, /**< The anchor rectangle's bottom edge. */
  LYR_ANCHOR_LEFT      = 4, /**< The anchor rectangle's left edge. */
  LYR_ANCHOR_RIGHT     = 8  /**< The anchor rectangle's right edge. */
} LYR_Anchor;

/**
 * Possible keyboard focus modes.
 *
 * \since This enum is available since SDL_layer 1.0.0.
 *
 * \sa LYR_CreateWindow
 * \sa LYR_PROP_WINDOW_CREATE_KEYBOARD_FOCUS_NUMBER
 */
typedef enum LYR_Focus {
  LYR_FOCUS_NONE       = 0, /**< Disable keyboard focus. */
  LYR_FOCUS_EXCLUSIVE  = 1, /**< Enable focus for top-most layer. */
  LYR_FOCUS_ON_DEMAND  = 2  /**< Enable regular focus semantics. */
} LYR_Focus;

/**
 * Initialize SDL_layer.
 *
 * This function requires SDL's video subsystem to be initialized using the
 * Wayland backend, calling it when already initialized is a no-op.
 *
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \since This function is available since SDL_layer 1.0.0.
 *
 * \sa LYR_Quit
 */
extern SDL_DECLSPEC bool SDLCALL LYR_Init(void);

/**
 * Deinitialize SDL_layer.
 *
 * Calling this function when already deinitialized is a no-op.
 *
 * \since This function is available since SDL_layer 1.0.0.
 */
extern SDL_DECLSPEC void SDLCALL LYR_Quit(void);

/**
 * Create a layer window with the specified properties.
 *
 * The window size and position are requests and may be adjusted by the
 * compositor. Margins are only meaningful for anchored edges. The exclusive
 * zone is also subject to compositor policy.
 *
 * These are the supported properties:
 *
 * - `SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER`: The desired width of the window,
 *   in screen coordinates.
 * - `SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER`: The desired height of the window,
 *   in screen coordinates.
 * - `LYR_PROP_WINDOW_CREATE_LAYER_NUMBER`: The layer on which to place the
 *   window, one of the values of `LYR_Layer`.
 * - `LYR_PROP_WINDOW_CREATE_ANCHOR_NUMBER`: A bitmask of `LYR_Anchor` values
 *   specifying which edges of the display the window should be anchored to.
 *   If no edges are specified, the window is centered.
 * - `LYR_PROP_WINDOW_CREATE_DISPLAY_NUMBER`: The SDL display ID on which to
 *   create the window. If not specified, the compositor chooses the display.
 * - `LYR_PROP_WINDOW_CREATE_NAMESPACE_STRING`: A namespace identifying the
 *   surface to the compositor. This can be used by the compositor to
 *   identify and configure surfaces belonging to a particular application.
 * - `LYR_PROP_WINDOW_CREATE_MARGIN_TOP_NUMBER`: The margin from the top edge
 *   of the display, in screen coordinates.
 * - `LYR_PROP_WINDOW_CREATE_MARGIN_RIGHT_NUMBER`: The margin from the right
 *   edge of the display, in screen coordinates.
 * - `LYR_PROP_WINDOW_CREATE_MARGIN_BOTTOM_NUMBER`: The margin from the bottom
 *   edge of the display, in screen coordinates.
 * - `LYR_PROP_WINDOW_CREATE_MARGIN_LEFT_NUMBER`: The margin from the left
 *   edge of the display, in screen coordinates.
 * - `LYR_PROP_WINDOW_CREATE_KEYBOARD_FOCUS_NUMBER`: The keyboard focus mode
 *   for the window, one of the values of `LYR_Focus`.
 * - `LYR_PROP_WINDOW_CREATE_EXCLUSIVE_ZONE_NUMBER`: The exclusive zone of
 *   the window, in screen coordinates. A positive value reserves space
 *   along the anchored edge so that other windows do not overlap it.
 *   A value of 0 indicates that no space should be reserved, and -1
 *   requests an exclusive zone extending to the edge of the display.
 * - `LYR_PROP_WINDOW_CREATE_EXCLUSIVE_EDGE_NUMBER`: The edge of the display
 *   against which the exclusive zone should be applied. One of the
 *   `LYR_Anchor` values. If not specified, the compositor determines the
 *   edge from the window's anchors.
 * - `LYR_PROP_WINDOW_CREATE_IGNORE_POINTER_BOOLEAN`: If true, the window
 *   does not receive pointer input.
 *
 * This function is only supported when using SDL's Wayland video backend.
 * For additional properties, see SDL_CreateWindowWithProperties.
 *
 * \since This function is available since SDL_layer 1.0.0.
 *
 * \sa SDL_DestroyWindow
 * \sa SDL_CreateProperties
 * \sa SDL_GetWindowProperties
 * \sa SDL_CreateWindowWithProperties
 */
extern SDL_DECLSPEC SDL_Window *SDLCALL LYR_CreateWindow(SDL_PropertiesID props);

#define LYR_PROP_WINDOW_CREATE_LAYER_NUMBER           "LYR.window.create.layer"
#define LYR_PROP_WINDOW_CREATE_ANCHOR_NUMBER          "LYR.window.create.anchor"
#define LYR_PROP_WINDOW_CREATE_DISPLAY_NUMBER         "LYR.window.create.display"
#define LYR_PROP_WINDOW_CREATE_NAMESPACE_STRING       "LYR.window.create.namespace"
#define LYR_PROP_WINDOW_CREATE_MARGIN_TOP_NUMBER      "LYR.window.create.margin.top"
#define LYR_PROP_WINDOW_CREATE_MARGIN_RIGHT_NUMBER    "LYR.window.create.margin.right"
#define LYR_PROP_WINDOW_CREATE_MARGIN_BOTTOM_NUMBER   "LYR.window.create.margin.bottom"
#define LYR_PROP_WINDOW_CREATE_MARGIN_LEFT_NUMBER     "LYR.window.create.margin.left"
#define LYR_PROP_WINDOW_CREATE_KEYBOARD_FOCUS_NUMBER  "LYR.window.create.keyboard_focus"
#define LYR_PROP_WINDOW_CREATE_EXCLUSIVE_ZONE_NUMBER  "LYR.window.create.exclusive_zone"
#define LYR_PROP_WINDOW_CREATE_EXCLUSIVE_EDGE_NUMBER  "LYR.window.create.exclusive_edge"
#define LYR_PROP_WINDOW_CREATE_IGNORE_POINTER_BOOLEAN "LYR.window.create.ignore_pointer"
#define LYR_PROP_WINDOW_LAYER_SURFACE_POINTER         "LYR.window.layer_surface"

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <SDL3/SDL_close_code.h>

#endif /* SDL_layer_h_ */
