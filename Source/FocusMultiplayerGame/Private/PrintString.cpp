#include "PrintString.h"

void DebugTools::PrintToScreen(float duration, FColor color, FString text)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, duration, color, text);
	}
}
