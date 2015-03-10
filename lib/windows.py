import pyglet
from pyglet import clock
import pyglet.gl as gl
from config import RENDER_SIZE


render_texture = None


def getRenderTexture():
    return render_texture


class Renderer(pyglet.window.Window):
    """
    Offscreen windows where render stuff
    """
    def __init__(self, visions):
        self.visions = visions

        W, H = RENDER_SIZE
        pyglet.window.Window.__init__(self,
                                      caption='(pineal renderer)',
                                      fullscreen=0,
                                      width=W,
                                      height=H,
                                      vsync=0,
                                      visible=0)

        gl.glEnable(gl.GL_BLEND)
        gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)

        gl.glEnable(gl.GL_LINE_SMOOTH)
        gl.glEnable(gl.GL_POLYGON_SMOOTH)
        gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST)
        gl.glHint(gl.GL_POLYGON_SMOOTH_HINT, gl.GL_NICEST)

        gl.glEnable(gl.GL_LIGHTING)
        gl.glEnable(gl.GL_LIGHT0)
        gl.glEnable(gl.GL_COLOR_MATERIAL)
        gl.glShadeModel(gl.GL_SMOOTH)

        self.texture = None

    def on_draw(self):
        self.clear()

        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        (w, h) = self.get_size()
        gl.glScalef(
            float(min(w, h))/w,
            -float(min(w, h))/h,
            1
        )

        gl.gluPerspective(45.0, 1, 0.1, 1000.0)
        gl.gluLookAt(0, 0, 2.4,
                     0, 0, 0,
                     0, 1, 0)

        global render_texture
        render_texture = self.texture

        for v in self.visions.values():
            gl.glMatrixMode(gl.GL_MODELVIEW)
            gl.glLoadIdentity()
            v.iteration()

        buf = pyglet.image.get_buffer_manager().get_color_buffer()
        rawimage = buf.get_image_data()
        self.texture = rawimage.get_texture()

        clock.tick()

    def update(self):
        self.switch_to()
        self.dispatch_events()
        self.dispatch_event('on_draw')
        self.flip()
        print '\rfps: %3.1f' % clock.get_fps(),


class Master(pyglet.window.Window):
    """
    Master output, on secondary monitor is exists, hidden otherwise
    """
    def __init__(self):
        platform = pyglet.window.get_platform()
        display = platform.get_default_display()
        screens = display.get_screens()

        W, H = RENDER_SIZE
        if len(screens) > 1:
            pyglet.window.Window.__init__(self,
                                          caption='(pineal master)',
                                          fullscreen=1,
                                          screen=screens[-1],
                                          vsync=1,
                                          visible=1)
            self.set_mouse_visible(False)
        else:
            pyglet.window.Window.__init__(self,
                                          caption='(pineal master)',
                                          fullscreen=0,
                                          width=W,
                                          height=H,
                                          vsync=1,
                                          visible=0)

        self.textures = []

    def on_draw(self):
        w, h = self.width, self.height
        side = max(w, h)

        gl.glTexParameteri(gl.GL_TEXTURE_2D,
                           gl.GL_TEXTURE_MAG_FILTER,
                           gl.GL_NEAREST)

        self.clear()
        for text in self.textures:
            text.blit(-(side-w)/2,
                      -(side-h)/2,
                      0,
                      side, side)

    def update(self, textures):
        self.textures = textures
        self.switch_to()
        self.dispatch_events()
        self.dispatch_event('on_draw')
        self.flip()


class Overview(pyglet.window.Window):
    """
    Overview for the programmer, nothing else to say
    """
    def __init__(self):

        pyglet.window.Window.__init__(self,
                                      resizable=True,
                                      caption='(pineal overview)',
                                      width=600, height=450,
                                      vsync=0)
        self.textures = []
        # self.fps_display = pyglet.clock.ClockDisplay()  # segfaults

    def on_draw(self):
        w, h = self.width, self.height
        side = max(w, h)

        self.clear()
        for text in self.textures:
            text.blit(-(side-w)/2,
                      -(side-h)/2,
                      0,
                      side, side)
        # self.fps_display.draw()

    def update(self, textures):
        self.textures = textures
        self.switch_to()
        self.dispatch_events()
        self.dispatch_event('on_draw')
        self.flip()
