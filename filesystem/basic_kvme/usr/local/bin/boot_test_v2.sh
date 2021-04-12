#!/bin/sh

test_DQS0()
{
	io 0 0x1e720008 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "DQS0(0x1e720008)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 3) { result = -1; return }
		# Append a "0" if string length is 3 only
		if (val_str_len == 3) { val_str = "0"val_str; val_str_len += 1 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		min = sprintf("0x%s", substr(val_str, val_str_len-1, 2))+0
		max = sprintf("0x%s", substr(val_str, val_str_len-3, 2))+0
		# Pass criteria is > 14
		result = ((max-min) > 14)?(0):(-1)
		printf "max=%d, min=%d, diff=%d, %s\n", max, min, max-min, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

test_DQS1()
{
	io 0 0x1e72000c | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "DQS1(0x1e72000c)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 3) { result = -1; return }
		# Append a "0" if string length is 3 only
		if (val_str_len == 3) { val_str = "0"val_str; val_str_len += 1 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		min = sprintf("0x%s", substr(val_str, val_str_len-1, 2))+0
		max = sprintf("0x%s", substr(val_str, val_str_len-3, 2))+0
		# Pass criteria is > 14
		result = ((max-min) > 14)?(0):(-1)
		printf "max=%d, min=%d, diff=%d, %s\n", max, min, max-min, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

test_CBR_retry()
{
	io 0 0x1e7200a0 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "CBR Retry Cnt(0x1e7200a0)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 1) { result = -1; return }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		cnt = sprintf("0x%s", val_str)+0
		# Pass criteria is < 3
		result = (cnt < 3)?(0):(-1)
		printf "cnt=%d %s\n", cnt, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

init_adc()
{
	# ToDo. DON'T use this function!!!!!!!!
	# It will disable the other HW unexpectedly.
	# Should write a ADC driver instead.

	#io 0 1e6e2004 clear bit[23] to enable ADC
	io 1 1e6e2004 ff2757f8
	# enable ADC by set bit[16:20] and bit[0:3]
	io 1 1e6e9000 1f000f
}

test_core_v()
{
	# Get ADC0 
	# io 0 1e6e9010 [0:9]

	io 0 0x1e6e9010 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "ADC0[ 0: 9](0x1e6e9010)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 1) { result = -1; return }
		# Append a "0" if string length is less then 7
		if (val_str_len < 7) { val_str = "00000"val_str; val_str_len += 5 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		val = sprintf("0x%s", substr(val_str, val_str_len-2, 3))+0
		# Convert to Voltage.
		val *= 1000
		val = (val * (2.5 * 1000)) / (1023 * 1000)
		# Pass criteria is 1.38V +/- 0.05V
		diff = val - (1.38 * 1000)
		diff = (diff < 0)?(-diff):(diff)
		result = (diff < (0.05 * 1000))?(0):(-1)
		printf "Core_V=%2.3fV diff=%2.3fV %s\n", val/1000, diff/1000, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

test_ddr2_v()
{
	# Get ADC1 
	#io 0 1e6e9010 [16:25]

	io 0 0x1e6e9010 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "ADC1[16:25](0x1e6e9010)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 1) { result = -1; return }
		# Append a "0" if string length is less then 7
		if (val_str_len < 7) { val_str = "00000"val_str; val_str_len += 5 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		val = sprintf("0x%s", substr(val_str, val_str_len-6, 3))+0
		# Convert to Voltage.
		val *= 1000
		val = (val * (2.5 * 1000)) / (1023 * 1000)
		# Pass criteria is 1.886V +/- 0.1V
		diff = val - (1.886 * 1000)
		diff = (diff < 0)?(-diff):(diff)
		result = (diff < (0.1 * 1000))?(0):(-1)
		printf "DDR2_V=%2.3fV diff=%2.3fV %s\n", val/1000, diff/1000, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

test_3d3_v()
{
	# Get ADC2 
	# io 0 1e6e9014 [0:9]

	io 0 1e6e9014 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "ADC2[ 0: 9](0x1e6e9014)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 1) { result = -1; return }
		# Append a "0" if string length is less then 7
		if (val_str_len < 7) { val_str = "00000"val_str; val_str_len += 5 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		val = sprintf("0x%s", substr(val_str, val_str_len-2, 3))+0
		# Convert to Voltage.
		val *= 1000
		val = (val * (2.5 * 1000)) / (1023 * 1000)
		# val = val * (R1+R2) / R2. Where R1==10K, R2==10K
		val = val * 2
		# Pass criteria is 3.3V +/- 0.3V
		diff = val - (3.3 * 1000)
		diff = (diff < 0)?(-diff):(diff)
		result = (diff < (0.3 * 1000))?(0):(-1)
		printf "3d3_V=%2.3fV diff=%2.3fV %s\n", val/1000, diff/1000, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

test_5d0_v()
{
	# Get ADC3 
	# io 0 1e6e9014 [16:25]

	io 0 1e6e9014 | awk '
	{
		val_str = $3""
		val_str_len = length(val_str)
		printf "ADC3[16:25](0x1e6e9014)=0x%s\t", val_str
		# Check invalid string length
		if (val_str_len < 1) { result = -1; return }
		# Append a "0" if string length is less then 7
		if (val_str_len < 7) { val_str = "00000"val_str; val_str_len += 5 }
		# awk treat numbers as signed value. 
		# For the case of 0x80000000, it will be treated as negative value.
		# So, we get the min and max value by substr. (string operation)
		val = sprintf("0x%s", substr(val_str, val_str_len-6, 3))+0
		# Convert to Voltage.
		val *= 1000
		val = (val * (2.5 * 1000)) / (1023 * 1000)
		# val = val * (R1+R2) / R2. Where R1==27K, R2==15K
		val = val * 2.8
		# Pass criteria is 5V +/- 0.5V
		diff = val - (5 * 1000)
		diff = (diff < 0)?(-diff):(diff)
		result = (diff < (0.5 * 1000))?(0):(-1)
		printf "5d0_V=%2.3fV diff=%2.3fV %s\n", val/1000, diff/1000, (result == 0)?("PASS"):("FAIL")
	}
	END {
		#printf "result=%d\n", result
		exit result
	}
	'
}

boot_up_test()
{
	_r='PASS'
	echo "------------------------------------------------------------------"
	echo "Start boot up test for SoC V2"
	# Test 1. Read DLL DQS0, 0x1e720008 bit[15:8] - bit[7:0] > 15
	if ! test_DQS0 ; then
		_r='FAIL'
	fi
	# Test 2. Read DLL DQS1, 0x1e72000c bit[15:8] - bit[7:0] > 15
	if ! test_DQS1 ; then
		_r='FAIL'
	fi
	# Test 3. CBR retry count, 0x1e7200a0 < 3
	if ! test_CBR_retry ; then
		_r='FAIL'
	fi
	# Test 4. Voltage Check
	#init_adc
	# ADC0 == 1.38V +/- 0.05V
	#if ! test_core_v ; then
	#	_r='fail' # Ignore fail for now (ADC issue)
	#fi
	# ADC1 == 1.886V +/- 0.1V
	#if ! test_ddr2_v ; then
	#	_r='fail' # Ignore fail for now (ADC issue)
	#fi
	# ADC2 == 3.3V +/- 0.3V
	#if ! test_3d3_v ; then
	#	_r='fail' # Ignore fail for now (ADC issue)
	#fi
	# ADC3 == 5V +/- 0.5V
	#if ! test_5d0_v ; then
	#	_r='fail' # Ignore fail for now (ADC issue)
	#fi
	echo "------------------------------------------------------------------"
	if [ "$_r" = 'FAIL' ]; then
		exit 1
	fi
}

boot_up_test
exit 0
