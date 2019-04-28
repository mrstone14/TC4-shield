// SGfilters.ino
// ------------
//
// this code is part of the TC4ESP project, a fork of aArtisanQ_PID TC4 
// signal processing improvement: Savitsky-Golay filters for RoR computation and filtering
//
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2018-2019
// All rights reserved.
//
// Author:  Renato Aranghelovici
//
// Redistribution and use in source and binary forms, with or without modification, are 
// permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this list of 
//   conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this list 
//   of conditions and the following disclaimer in the documentation and/or other materials 
//   provided with the distribution.
//
//   Neither the name of the copyright holder(s) nor the names of its contributors may be 
//   used to endorse or promote products derived from this software without specific prior 
//   written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------------------------------------------------------------------

// Revision history:
// 20190103 added RoR computation as profile first derivative using Savitsky-Golay filter
// https://en.wikipedia.org/wiki/Savitzky-Golay_filter#Tables_of_selected_convolution_coefficients
// 20190314: added 11 and 15 seconds window coefficients// 20190428: split into a separate file, for better portability/compatibility
#ifdef useSGfilter

//////////////////////////////////////////////////
// Custom CalcRise, based on Savitsky-Golay filter
//////////////////////////////////////////////////

// SG first deriv. coefficients
// coefficients for 7, 9, 11, 15 seconds window are provided, choose your preference
const int16_t sg_1stDer_coef[sgWindow] =
//{22, -67, -58, 0, 58, 67, -22}
//{86, -142, -193, -126, 0, 126, 193, 142, -86}
{ 300, -294, -532, -503, -296, 0, 296, 503, 532, 294, -300 }
//{ 12922, -4121, -14150, -18334, -17842, -13843, -7506, 0, 7506, 13843, 17842, 18334, 14150, 4121, -12922}
;

const int16_t sgNormalization = 5148;
// { 252 for 7 values window, 1188 for 9, 5148 for 11, 334152 for 15 };

float h = looptime / 1000; 

// temperatures F 
// time marks, milliseconds
int32_t sgTimesHist[NC][sgWindow];
float sgTempsHist[NC][sgWindow];


float sgRoR(uint8_t k, float Temp, int32_t time)
// perform RoR computation as Savitsky-Golay filter first derivative 
{

	uint8_t i;

	// shift history one position
	for (i = 1; i < sgWindow; i++)
	{
		sgTimesHist[k][i - 1] = sgTimesHist[k][i];
		sgTempsHist[k][i - 1] = sgTempsHist[k][i];
		sgRoRHist[k][i - 1] = sgRoRHist[k][i];
	}

	//histNextIdx++;
	//if histNextIdx > sgWindow) histNextIdx = 0;
	// add current values to history
	sgTimesHist[k][sgWindow - 1] = time;
	sgTempsHist[k][sgWindow - 1] = convertUnits((float)Temp);
	//Serial.print(sgWindow - 1); Serial.print(-sgTempsHist[k][sgWindow - 1]); Serial.println();

	if (sgTempsHist[k][sgWindow - 2] == 0) return 0; // empty history

	float dT;
	if (sgTempsHist[k][0] == 0) { // incomplete history, perform simple differential until history is filled
		int32_t dt = time - sgTimesHist[k][sgWindow - 2];
		if (dt == 0) return 0.0;  // fixme -- throw an exception here?
		dT = sgTempsHist[k][sgWindow - 1] - sgTempsHist[k][sgWindow - 2];
		float dS = dt * 0.001; // convert from milli-seconds to seconds
		return (dT / dS) * 60.0; // rise per minute
	}
	else { // history is full, preform SG first deriv calculus
		float sum = 0;
		for (i = 0; i < sgWindow; i++)
		{
			//Serial.print(i); Serial.print(" = "); Serial.print(sgTempsHist[k][i]); Serial.print(" * "); Serial.println(sg_1stDer_coef[i]);
			sum += sgTempsHist[k][i] * sg_1stDer_coef[i];
			//Serial.print(i); Serial.println(-sgTempsHist[k][i]);
			//Serial.print("sum: ");  Serial.println(sum);
		}
		//Serial.print("sum: ");  Serial.println(sum);
		h = (sgTimesHist[k][sgWindow - 1] - sgTimesHist[k][0]) / (sgWindow - 1);
		if (abs(looptime - h) > 50) { Serial.print("# h: ");  Serial.println(h); }
		h *= 0.001;
		dT = sum / (sgNormalization * h);
	}

	dT = dT * 60.0; // rise per minute
	if (dT > 999) dT = 999;
	if (dT < -999) dT = -999;
	sgRoRHist[k][sgWindow - 1] = dT;
	//Serial.print("dT: ");  Serial.println(dT);
	return dT; // rise per minute

}

// End of SG RoR

#endif // useSGfilter
