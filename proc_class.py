class Proc:
    def __init__(self):
        self.n = 0

    def initialize(self):
        #print("initialize>>>> self=", self)
        self.n += 1
        return 'initialize-' + str(self.n)

    def computeTile(self, x, y, array):
        #print("computeTile>>>> self =", self, ", x =", x, ", y =", y)
        self.n += 1
        return 'computeTile-' + str(self.n)

    def dispose(self):
        self.n += 1
        return 'dispose-' + str(self.n)




