
class Controller:
    def __init__(self, time):
        self.setPoint = 0
        self.integral = 0
        self.time = time

        self.lastError = 0

        self.kp = -0.1
        self.ki = -1.0
        self.kd = 0.0

    def process(self, response, time):
        delta = time - self.time
        self.time = time
        
        error = self.setPoint - response
        self.integral += error*delta
        diff = (error - self.lastError)/delta
        self.lastError = error

        control = self.kp*error + self.ki*self.integral + self.kd*diff
        control = max(0, min(control, 30))

        return (control, response)

