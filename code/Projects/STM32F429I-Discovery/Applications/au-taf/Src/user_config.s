	  .syntax unified
	  .cpu cortex-m4
	  .fpu softvfp
	  .thumb

	.section  .usercfg,"ad"
	.global  user_config
	.align	4

user_config:
	.hword	0
	.hword	0
	.hword	0
	.hword	0
	.hword	0
	.hword	0
	.hword	0
	.byte	'Z'
	.byte	'Z'
	.rept	16384-16
	.byte 	0
	.endr
