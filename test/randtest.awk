# this file was generated from the randtest.sh by the following sed:
# sed -n -e '/BEGIN/!d' -e 's/.*/BEGIN{ /' -e ':b' -e "s/'$\([^']*\)'/\1/" -e '/^'"'"'/q' -e 'p;n;b b' randtest.sh
BEGIN{ 
    srand(RANDOM);
    nsamples=(NSAMPLES);
    max_allowed_sigma=(MAX_ALLOWED_SIGMA);
    nruns=(NRUNS);
    for(tau=0;tau<nsamples/2;tau++) corr[tau]=0;

    for(run=0;run<nruns;run++) {
	sum=0;

	# Fill an array with a sequence of samples that are a
	# function of pairs of rand() values.

	for(i=0;i<nsamples;i++) {
	   samp[i]=((rand()-0.5)*(rand()-0.5))^2;
	   sum=sum+samp[i];
	   }

	# Subtract off the mean of the sequence:

	mean=sum/nsamples;
	for(i=0;i<nsamples;i++) samp[i]=samp[i]-mean;

	# Calculate an autocorrelation function on the sequence.
	# Because the values of rand() should be independent, there
	# should be no peaks in the autocorrelation.

	for(tau=0;tau<nsamples/2;tau++) {
	    sum=0;
	    for(i=0;i<nsamples/2;i++) sum=sum+samp[i]*samp[i+tau];
	    corr[tau]=corr[tau]+sum;
	    }

	}
    # Normalize the autocorrelation to the tau=0 value.

    max_corr=corr[0];
    for(tau=0;tau<nsamples/2;tau++) corr[tau]=corr[tau]/max_corr;

    # OPTIONALLY Print out the autocorrelation values:

    # for(tau=0;tau<nsamples/2;tau++) print tau, corr[tau] > "pairpower_corr.data";

    # Calculate the sigma for the non-zero tau values: 

    power_sum=0;

    for(tau=1;tau<nsamples/2;tau++) power_sum=power_sum+(corr[tau])^2;

    sigma=sqrt(power_sum/(nsamples/2-1));

    # See if any of the correlations exceed a reasonable number of sigma:

    passed=1;
    for(tau=1;tau<nsamples/2;tau++) {
	if ( abs(corr[tau])/sigma > max_allowed_sigma ) {
	    print "Tau=", tau ", Autocorr=", corr[tau]/sigma, "sigma";
	    passed=0;
	    }
        }
    if(!passed) {
	print "Test failed."
	exit(1);
        }
    else exit (0);
    }

function abs(abs_input) { return(sqrt(abs_input^2)) ; }
