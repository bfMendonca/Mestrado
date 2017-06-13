import numpy as np

class LantecyCalc( object ):

	def __init__(self, file_name ):

		#Inicializando as variaveis
		self.periods = np.array( [])
		self.mean = 0.0
		self.std_deviation = 0.0
		self.max = 0.0
		self.size = 0

		#Iterando nas linhas do arquivo e lendo os periodos.
		file = open( file_name, 'r' )
		for line in file:
			period = float(line) / 1000.0

			if period > self.max:
				self.max = period
				pass

			self.periods = np.append( self.periods, period)
            
        # Com os dados acima, savando algumas caracteristicas de interesse
		self.mean = self.periods.mean()
		self.std_deviation = self.periods.std()
		self.max = self.periods.max()
		self.size = len( self.periods )
		self.outliers = ( ( self.periods > (self.mean + 3*self.std_deviation) ) | ( self.periods < (self.mean - 3*self.std_deviation) ) ).sum()