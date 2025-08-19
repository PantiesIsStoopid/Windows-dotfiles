
#Requires AutoHotkey v2.0

^!m:: {
  ; Get active window position
  WinGetPos(&X, &Y, &W, &H, "A")
  CenterX := X + W/2
  CenterY := Y + H/2

  ; Find which monitor the window is on
  MonCount := MonitorGetCount()
  ActiveMon := 1
  Loop MonCount {
    MonitorGetWorkArea(A_Index, &L, &T, &R, &B)
    if (CenterX >= L && CenterX <= R && CenterY >= T && CenterY <= B) {
      ActiveMon := A_Index
      Break
    }
  }

  ; Get active monitor work area
  MonitorGetWorkArea(ActiveMon, &L, &T, &R, &B)

  ; Window size = 70% of monitor
  SizeRatio := 0.7
  NewW := Floor((R - L) * SizeRatio)
  NewH := Floor((B - T) * SizeRatio)

  ; Center inside that monitor
  NewX := L + Floor(((R - L) - NewW) / 2)
  NewY := T + Floor(((B - T) - NewH) / 2)

  ; Move/resize active window
  WinMove(NewX, NewY, NewW, NewH, "A")
}
