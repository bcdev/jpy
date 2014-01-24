class Processor:
    def __init__(self):
        self.n = 0


    def initialize(self):
        return 'initialize'

    def computeTile(self, w, h, array):
        self.n += 1
        self.spend_some_time()
        return 'computeTile-' + str(self.n) + '-' + str(w) + ',' + str(h)

    def dispose(self):
        return 'dispose'


    def spend_some_time(self):
        l = list(range(10000))
        for i in range(10000):
            l.reverse()



