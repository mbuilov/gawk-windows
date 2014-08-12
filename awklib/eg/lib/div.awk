# div --- do integer division

#
# Arnold Robbins, arnold@skeeve.com, Public Domain
# July, 2014

function div(numerator, denominator, result,    i)
{
    split("", result)

    numerator = int(numerator)
    denominator = int(denominator)
    result["quotient"] = int(numerator / denominator)
    result["remainder"] = int(numerator % denominator)

    return 0.0
}
