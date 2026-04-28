#pragma once
#ifndef PrintStrings
#define PrintStrings

namespace DebugTools
{
	void PrintToScreen(float duration, FColor color, FString text);
}

#define PrintFormatted(Format, ...) if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT(Format), ##__VA_ARGS__))

#endif