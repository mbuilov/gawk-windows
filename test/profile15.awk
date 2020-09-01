# in namespace awk
function zzzz()
{
	return "zzz"
}

function nnn()
{
	return "nnn"
}

@namespace "nnn"
function abc()
{
	return "nnn::abc"
}

@namespace "bbb"

function zzz()
{
	return "bbb::zzz"
}

