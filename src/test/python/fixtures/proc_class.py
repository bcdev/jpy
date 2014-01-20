class Processor:
    def __init__(self):
        self.n = 0

    def initialize(self):
        self.n += 1
        return 'initialize-' + str(self.n)

    def computeTile(self, w, h, array):
        self.n += 1
        return 'computeTile-' + str(self.n) + '-' + str(w) + ',' + str(h)

    def dispose(self):
        self.n += 1
        return 'dispose-' + str(self.n)




