/*	File:		MovieProc.c	Contains:	Code demonstrating how to use a movie drawing callback procedure	Written by: Apple Developer Technical support	Copyright:	Copyright � 2003 by Apple Computer, Inc., All Rights Reserved.	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.	Change History (most recent first):				3/18/2003	srk				Carbonized				8/17/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1				*/#include "MovieProc.h"#include <Fonts.h>// GLOBALSMovie 					gMovie = NULL;WindowRef 				gWindow = NULL;Rect					gMovieRect = {0, 0, 0, 0};TimeValue				gMovieDuration = 0L;long					gTickCount = 0L;long					gSampleCount = 0L;Rect					gTimeDurationRect = { 5, 5, 10 ,110};MovieDrawingCompleteUPP	gMovieProc = NULL;// ______________________________________________________________________//�  MAIN -- Starting point.int main(void) {	EventRecord anEvent;	Boolean     fpsStatsDrawn = false;		//� Initialize the needed parts.	InitMacEnvironment(6);	InitializeQTEnvironment();	if ( InitializeMovie() != noErr ) ExitToShell();		// we had problems with the movie.	//� Draw basic strings inside the window.	DrawFpsStats(0L); 	DrawMovieFpsStats();	DrawUsageInformation();		gTickCount = TickCount();	gSampleCount = 1L;	//� Event loop.	for(;;) {		WaitNextEvent(everyEvent, &anEvent, 1, NULL);		MoviesTask(gMovie, 0);				if (IsMovieDone(gMovie))		{			if (!fpsStatsDrawn)		// draw fps stats only once			{				fpsStatsDrawn = true;								gTickCount = TickCount() - gTickCount;				DrawFpsStats(gTickCount);				QTUDrawVideoFrameAtTime(gMovie, GetMovieTime(gMovie, 0));  // nudge one last time to make sure last frame is drawn.			}		}		if(anEvent.what == mouseDown)			break;		if(anEvent.what == keyDown) {			DrawFpsStats(0L); 			fpsStatsDrawn = false;						GoToBeginningOfMovie(gMovie); 			gTickCount = TickCount();			gSampleCount = 1L;			StartMovie(gMovie); 		}			}		return 0;}// ______________________________________________________________________//� InitMacEnvironment -- Initialize the Mac Toolbox.void InitMacEnvironment(long nMasters) {	long i;	for(i = 0; i <nMasters; i++)		MoreMasters();		FlushEvents(everyEvent, 0);	InitCursor();}// ______________________________________________________________________//� InitializeQTEnvironment -- Initialize the QuickTime movie toolbox parts.void InitializeQTEnvironment(void) {	OSErr anErr = noErr;		if( !QTUIsQuickTimeInstalled() ) {		DebugStr("\pThe QuickTime extension is not present in this system");		ExitToShell();	}	if( (QTUGetQTVersion() >> 16 ) < 0x200 ) {		DebugStr("\pWe need QT 2.0 or higher due to APIs used in this sample, consult the sources (exit).");		ExitToShell();	}	#if powerc		if( !QTUIsQuickTimeCFMInstalled() ) {		DebugStr("\pThe QuickTime PowerPlug extension is not available (exit)");		ExitToShell();		}							#endif 	anErr = EnterMovies(); DebugAssert(anErr == noErr);	if(anErr != noErr) {		DebugStr("\pProblems with Entermovies, returning errors (exit)");		ExitToShell();	}}// ______________________________________________________________________//� InitializeMovie -- Initialize needed movie parts for the offscreen handling.OSErr InitializeMovie(void) {	OSErr 		anErr = noErr;	Rect 		windowBounds = { kWindowYStart, kWindowXStart, kWindowHeigth, kWindowLength};	CGrafPtr	aSavedPort = NULL;	GDHandle	aSavedGDevice = NULL;		//� Create the window we will use.	anErr = CreateNewWindow( kDocumentWindowClass, 							kWindowCloseBoxAttribute, 							&windowBounds, 							&gWindow ); DebugAssert(anErr == noErr);	ShowWindow(gWindow);    // set the port to the new window	SetPortWindowPort(gWindow);	GetGWorld(&aSavedPort, &aSavedGDevice);	//� Get the movie.	anErr = QTUSimpleGetMovie(&gMovie); DebugAssert(anErr == noErr);	if(anErr) goto Closure;		//� Adjust the movie box values.	GetMovieBox(gMovie, &gMovieRect); 	OffsetRect(&gMovieRect,  -gMovieRect.left,  -gMovieRect.top);	SetMovieBox(gMovie, &gMovieRect); 	AlignWindow(gWindow, false,  &gMovieRect, NULL);	//� Specify the Movie GWorld.	SetMovieGWorld(gMovie, GetWindowPort(gWindow), NULL);	anErr = GetMoviesError(); DebugAssert(anErr == noErr); if(anErr) goto Closure;		//� Install the movie drawing complete callback.	gMovieProc = NewMovieDrawingCompleteUPP(&MyQTMovieDrawingCompleteProc);	SetMovieDrawingCompleteProc(gMovie, movieDrawingCallWhenChanged, gMovieProc, 0);	anErr = GetMoviesError(); DebugAssert(anErr == noErr); if(anErr) goto Closure;		//� Get movie duration.	gMovieDuration = GetMovieDuration(gMovie);	Closure:	return anErr;}// ______________________________________________________________________//� MyTrackTransferProc -- Callback called when movie toolbox draws to GWorld.pascal OSErr MyQTMovieDrawingCompleteProc(Movie theMovie, long refCon) {#pragma unused (theMovie)#pragma unused (refCon)	OSErr 			anErr = noErr;	Str255			tempString;	long			percentage = 0L;	Rect			eraseRect = { 12, kDrawTextX - 5, 24, kDrawTextX + 25};	//� Bounce the sample code (needed for later statistics).	gSampleCount++;		//� Draw progress bar into main screen. 	PenSize(1,1); ForeColor(blackColor); FrameRect(&gTimeDurationRect);	percentage = 100L * GetMovieTime(gMovie, NULL) / gMovieDuration;	MoveTo(5,5); ForeColor(yellowColor); PenSize(4,4); LineTo( percentage +5L, 5); 	//� Draw percentage numbers.	EraseRect(&eraseRect);	MoveTo(kDrawTextX, 20); TextFace(bold); TextSize(9); 	NumToString(percentage, tempString);  ForeColor(redColor); DrawString(tempString); 	MoveTo(kDrawTextX + 20, 20); DrawString("\p%"); ForeColor(blackColor);	return anErr;}// ______________________________________________________________________//� DrawFpsStats -- Provide and draw statistics concerning how many frames were drawn per second.void DrawFpsStats(long tickCount){	Str255 	theString;	Rect	eraseArea = {kDrawValuesY-20, kDrawValuesX -20, kDrawValuesY+20, kDrawValuesX+20};	EraseRect(&eraseArea); 		//� Display the fps values.	MoveTo(kDrawTextX, kDrawTextY); 	TextFace(bold); TextSize(9);	DrawString("\pFrames drawn/second:");	if(tickCount != 0L) {		MoveTo(kDrawValuesX, kDrawValuesY);		NumToString( (60L * gSampleCount/tickCount), theString); 		DrawString(theString);	}}// ______________________________________________________________________//� DrawMovieFpsStats -- Get the statistics from the movie concerning frames per second, draw this.void DrawMovieFpsStats(void) {	Str255	theString;	long	nFrames = 0L;	long	nSeconds = 0L;		MoveTo(kDrawTextX, kDrawTextY+40);	TextFace(bold); TextSize(9);	DrawString("\pMovie stats, fps: ");	MoveTo(kDrawValuesX, kDrawValuesY + 40);	nFrames =  gMovieDuration / QTUGetDurationOfFirstMovieSample(gMovie, VideoMediaType);	nSeconds = gMovieDuration / GetMovieTimeScale(gMovie);	NumToString( (nFrames/nSeconds), theString);	DrawString(theString);}// ______________________________________________________________________//� DrawUsageInformation -- Draw instructions how to use this simple program.void DrawUsageInformation(void) {	TextFace(normal); TextSize(9);	MoveTo(kDrawTextX, 200); DrawString("\pHit a key to start the movie.");	MoveTo(kDrawTextX, 220); DrawString("\pClick on the mouse to terminate program.");}