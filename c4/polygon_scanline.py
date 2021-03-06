from PIL import Image
import math


class Polygon:

    def __init__(self, points, pixels):
        print(points)
        self.points = points
        self.points.append(points[0])
        self.pixels = pixels
        self.edges = []
        self.edge_table = {}
        self.miny = None
        self.maxy = None

        for i in range(1, len(points)):
            x1, y1 = points[i - 1]
            x2, y2 = points[i]
            if y2 < y1:
                x1, y1, x2, y2 = x2, y2, x1, y1

            if self.miny is None or y1 < self.miny:
                self.miny = y1
            if self.maxy is None or y2 > self.maxy:
                self.maxy = y2

            d = 0
            if y1 != y2:
                d = (x1 - x2) / (y1 - y2)
            edge = {
                'ymax': y2,
                'x': x1,
                'deltax': d,
            }

            self.edges.append(edge)
            if y1 not in self.edge_table.keys():
                self.edge_table[y1] = []
            self.edge_table[y1].append(edge)

    def draw(self):
        y = self.miny
        ael = []
        while True:
            if y in self.edge_table.keys():
                ael += self.edge_table[y]
                ael.sort(key=lambda e: e['x'])
            for e in list(ael):
                if e['ymax'] == y:
                    ael.remove(e)
            for i in range(0, len(ael) - 1, 2):
                sx = int(ael[i]['x'])
                dx = int(ael[i + 1]['x'])
                if sx > dx:
                    sx, dx = dx, sx
                for x in range(sx, dx):
                    self.pixels[x, y] = (255, 0, 0)
            y += 1
            for e in ael:
                e['x'] += e['deltax']
            if len(ael) == 0:
                break


class PolygonApp:

    _WIDTH = 200
    _HEIGHT = 200

    def __init__(self):
        self.img = Image.new('RGB', (PolygonApp._WIDTH, PolygonApp._HEIGHT), 'black')
        self.pixels = self.img.load()

        print("请输入点数：")
        n = int(input())
        points = []
        print("一行一个点，空格隔开x和y(画布大小200*200)")
        for i in range(0, n):
            x, y = input().split()
            x, y = int(x), int(y)
            points.append((x, y))

        self.polygon = Polygon(points, self.pixels)
        self.polygon.draw()
        self.img.show()

if __name__ == '__main__':
    app = PolygonApp()

