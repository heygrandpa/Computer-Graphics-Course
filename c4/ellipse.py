from PIL import Image
import math


class Ellipse:

    def __init__(self, x, y, a, b, pixels):
        self.x = x
        self.y = y
        self.a = a
        self.b = b
        self.pixels = pixels

    def draw_pixels(self, sx, sy):
        self.pixels[self.x + sx, self.y + sy] = (255, 0, 0)
        self.pixels[self.x + sx, self.y - sy] = (255, 0, 0)
        self.pixels[self.x - sx, self.y + sy] = (255, 0, 0)
        self.pixels[self.x - sx, self.y - sy] = (255, 0, 0)

    def draw(self):
        x = self.x
        y = self.y
        a = self.a
        b = self.b

        dis_ab = math.sqrt(a ** 2 + b ** 2)
        tx = a * a / dis_ab
        ty = b * b /dis_ab

        sx = 0
        sy = b
        d = 4 * b**2 - 4 * b * a**2 + a**2

        while sx <= tx:
            self.draw_pixels(sx, sy)
            if d <= 0:
                d += 4 * b**2 * (2 * sx + 3)
            else:
                d += 4 * b**2 * (2 * sx + 3) - 8 * a**2 * (sy - 1)
                sy -= 1
            sx += 1

        sx = a
        sy = 0
        d = 4 * a**2 - 4 * a * b**2 + b**2

        while sy < ty:
            self.draw_pixels(sx, sy)
            if d <= 0:
                d += 4 * a**2 * (2 * sy + 3)
            else:
                d += 4 * a**2 * (2 * sy + 3) - 8 * b**2 * (sx - 1)
                sx -= 1
            sy += 1


class EllipseApp:

    _WIDTH = 640
    _HEIGHT = 480

    def __init__(self):
        self.img = Image.new('RGB', (EllipseApp._WIDTH, EllipseApp._HEIGHT), 'white')
        self.pixels = self.img.load()
        self.ellipse = Ellipse(EllipseApp._WIDTH / 2, EllipseApp._HEIGHT / 2, 200, 150, self.pixels)
        self.ellipse.draw()
        self.img.show()

if __name__ == '__main__':
    app = EllipseApp()

