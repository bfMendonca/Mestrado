#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import LatencyCalc as calc

def main():
	
	size_args = len( sys.argv )

	for i in range(1,size_args):
		filename = sys.argv[i]

		stats = calc.LantecyCalc( filename )

		##------ Exibindo as caracteristicas do arquivo. --------##
		print( "\n########################")
		print( "Caracteristicas do arquivo: ", filename )
		print( "Mediana:", stats.mean )
		print( "Desvio padrao:", stats.std_deviation )
		print( "Max:", stats.max)
		print( "Numero de amostras:", stats.size )
		print( "Numero de amostras com 3 desvios padrões acima da mediana:", stats.outliers )
		print( "########################\n")

if __name__ == '__main__':
	main()
