## watermark_disabler
A simple project made to disable the annoying "Activate Windows" watermark.

### approach #1
In win32kfull.sys, there's a function called PaintWatermark that renders the activation watermark, this function gets called by xxxDesktopPaintCallback:

```cpp
    if ( *(_DWORD *)(*(_QWORD *)gpsi + 0x874i64) )
    {
      v21 = *(_QWORD *)(*(_QWORD *)gptiCurrent + 0x1C0i64);
      if ( v21 )
        v22 = *(_QWORD *)(*(_QWORD *)(v21 + 8) + 0xA8i64);
      else
        v22 = 0i64;
      v13 = v22 == 0;
    }
    else
    {
      v13 = 0;
    }
    if ( v13 )
      PaintWatermark(v4, &v23);
```

Which can be simplified to:

```cpp
if ( gpsi->unk874h != 0 )
{
	/* gptiCurrent + 0x1c0 = tagDESKTOP** */
	const auto desktop = gptiCurrent->desktops[1]; /* type: tagDESKTOP**, this is checked if it's grpdeskLogon, which is a global pointer to the lock screen */
	
	HWND desktop_window = nullptr;
	
	/* tagDESKTOP + 0xa8 = tagWnd* */
	if ( desktop )
		desktop_window = desktop->wnd; /* type: tagWnd*, I believe this is a pointer to the lock window? */
	
	should_draw_watermark = ( desktop_window == nullptr );
}

if ( should_draw_watermark )
	PaintWatermark(device_context, &desktop_rect);
```

gpsi is a global pointer to a tagSERVERINFO structure, and gptiCurrent is a global pointer to a _THREADINFO structure.

As you can see from the snippets above, this: gpsi->unk874h is checked to be 1 before drawing the watermark, so, by forcing it to be 0, the checks will fail and the watermark won't be drawn.

### approach #2

PaintWatermark calls GreExtTextOutWInternal (which is the internal function for ExtTextOutW/NtGdiExtTextOutW in wingdi.h). 

The argument passed for size is a global called "gSafeModeStrLen", by setting the size to 0, the string won't be rendered. I left below a pattern for the global in win32kfull.

pattern: 44 8B C8 44 89 0D + 7
