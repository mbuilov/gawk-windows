@load "testext"

BEGIN {
	print ""
	print "test:", ARGV[1]
	switch (ARGV[1] + 0) {
	case 1:
		answer_num = 43
		break
	case 2:
		++answer_num
		break
	case 3:
		--answer_num
		break
	case 4:
		answer_num++
		break
	case 5:
		answer_num--
		break
	case 6:
		answer_num += 1
		break
	case 7:
		answer_num -= 1
		break
	case 8:
		answer_num *= 1
		break
	case 9:
		answer_num /= 1
		break
	case 10:
		answer_num ^= 1
		break
	case 11:
		answer_num = answer_num "foo"
		break
	case 12:
		sub(/2/, "3", answer_num)
		break
	case 13:
		a[1] = 1
		for (answer_num in a)
			print answer_num, a[answer_num]
		break
	case 14:
		test_func(answer_num)
		break
	}
}

function test_func(val)
{
	val++
	print "in test_func, val now =", val
}
