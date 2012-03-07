BEGIN {
	N = 22/7
	printf("   %.15f\n", N)

	printf("*  %.10f\n", N)	# default
	RNDMODE="N"; printf("N  %.10f\n", N)
	RNDMODE="U"; printf("U  %.10f\n", N)
	RNDMODE="D"; printf("D  %.10f\n", N)
	RNDMODE="Z"; printf("Z  %.10f\n", N)
	N = -N
	RNDMODE="N"; printf("N %.10f\n", N)
	RNDMODE="U"; printf("U %.10f\n", N)
	RNDMODE="D"; printf("D %.10f\n", N)
	RNDMODE="Z"; printf("Z %.10f\n", N)
}
