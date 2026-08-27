/* SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright 2026, Analog Devices, Inc. All rights reserved.
 */

int main(int argc, char **argv)
{
	/* Keep from constantly hitting the automatic main() breakpoint in
     * the while() loop below */
	asm("nop;");

	while (1) {
		asm("nop;");
	}
}
