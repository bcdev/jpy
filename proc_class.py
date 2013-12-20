class Proc:
    def __init__(self):
        self.n = 0

    def initialize(self):
        self.n += 1
        return 'initialize-' + str(self.n)

    def computeTile(self, x, y, array):
        self.n += 1
        return 'computeTile-' + str(n) + '-' + str(x) + ',' + str(y)

    def dispose(self):
        self.n += 1
        return 'dispose-' + str(self.n)




