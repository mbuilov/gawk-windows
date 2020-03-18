..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""find /v \\""\\""""}"

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""find /v \\""\\""""}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");close(""/dev/stdout"");print ""2nd""|""find /v \\""\\""""}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""find /v \\""\\"""";close(""find /v \\""\\"""")}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""find /v \\""\\"""";close(""find /v \\""\\"""")}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""find /v \\""\\"""";close(""find /v \\""\\"""")}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""sort""}"|find /v ""

..\gawk.exe "BEGIN{print ""1st"";fflush(""/dev/stdout"");print ""2nd""|""sort"";close(""sort"")}"|find /v ""
