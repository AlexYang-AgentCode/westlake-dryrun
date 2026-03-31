# WestlakeV3 — 179 API 全覆盖计划

> **核心逻辑**: 3 Demo 验证闭环 (45 API) → AI 复制模式铺开剩余 134 API → GitHub 自治
> **不是做完 3 个 Demo 就停。是验证完 AI 工作模式后，让 AI 自己完成全部。**

---

## 全景图

```
阶段 1: 3 Demo 验证闭环        45 API    人管意图 + AI 执行    Step 1-320
阶段 2: AI 全自动铺开          134 API   人只看报告           Step 321-990
阶段 3: GitHub 开源自治        179 API   Issue 驱动自动进化   Step 991+
         ────────────────────────────────────────────────────────
总计                           179 API   1人 + N个AI Agent
```

---

## 179 API 总表 (按 Plugin 分组)

### 01-Logging (4 API) — P1 | 阶段1 Demo1

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 1 | nativeLogDebug | Log.d() | OH_LOG_DEBUG | D1 |
| 2 | nativeLogInfo | Log.i() | OH_LOG_INFO | D1 |
| 3 | nativeLogWarn | Log.w() | OH_LOG_WARN | D1 |
| 4 | nativeLogError | Log.e() | OH_LOG_ERROR | D1 |

**参照**: str.h RAII 模式, WINE §原则1-2, Y-test/mock/

---

### 02-Activity (3 API) — P0 | 阶段1 Demo1+3

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 5 | nativeStartAbility | startActivity(Intent) | OH_AbilityManager_StartAbility(Want) | D1 |
| 6 | nativeTerminateSelf | finish() | OH_AbilityManager_TerminateSelf() | D1 |
| 7 | nativeCheckPermission | checkSelfPermission() | OH_AccessToken_VerifyAccessToken() | D1 |

**参照**: Y-shim/android/app/Activity.java, Y-skills/A2OH-LIFECYCLE.md, WINE §原则3 (Intent→Want两步转换)

---

### 03-Window (6 API) — P0 | 阶段1 Demo1

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 8 | nativeSurfaceCreate | SurfaceHolder.addCallback() | OH_NativeWindow_CreateNativeWindow() | D1 |
| 9 | nativeSurfaceDestroy | surfaceDestroyed() | OH_NativeWindow_DestroyNativeWindow() | D1 |
| 10 | nativeSurfaceResize | surfaceChanged(w,h) | OH_NativeWindow_NativeWindowSetSize() | D1 |
| 11 | nativeSurfaceGetCanvas | lockCanvas() | OH_NativeWindow_NativeWindowRequestBuffer() | D1 |
| 12 | nativeSurfaceFlush | unlockCanvasAndPost() | OH_NativeWindow_NativeWindowFlushBuffer() | D1 |
| 13 | nativeArkUIInit | N/A (WestLake专用) | OH_ArkUI_Init() | D1 |

**参照**: [H27 Waydroid] Virgl渲染, Y-bridge/ Surface方法

---

### 04-Rendering (47 API) — P0 | 阶段1 D1+D2, 阶段2 剩余

#### Canvas 基础 (12 API, Demo1)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 14 | nativeCanvasCreate | new Canvas(bitmap) | OH_Drawing_CanvasCreate() | D1 |
| 15 | nativeCanvasDrawColor | drawColor(color) | OH_Drawing_CanvasDrawColor() | D1 |
| 16 | nativeCanvasDrawRect | drawRect(l,t,r,b,paint) | OH_Drawing_CanvasDrawRect() | D1 |
| 17 | nativeCanvasDrawText | drawText(str,x,y,paint) | OH_Drawing_CanvasDrawText() | D1 |
| 18 | nativeCanvasSave | save() | OH_Drawing_CanvasSave() | D1 |
| 19 | nativeCanvasRestore | restore() | OH_Drawing_CanvasRestore() | D1 |
| 20 | nativeCanvasTranslate | translate(dx,dy) | OH_Drawing_CanvasTranslate() | D1 |
| 21 | nativeCanvasScale | scale(sx,sy) | OH_Drawing_CanvasScale() | D1 |
| 22 | nativeCanvasRotate | rotate(degrees) | OH_Drawing_CanvasRotate() | D1 |
| 23 | nativeCanvasClipRect | clipRect(l,t,r,b) | OH_Drawing_CanvasClipRect() | D2 |
| 24 | nativeCanvasDrawLine | drawLine(x1,y1,x2,y2) | OH_Drawing_CanvasDrawLine() | D2 |
| 25 | nativeCanvasDrawCircle | drawCircle(cx,cy,r) | OH_Drawing_CanvasDrawCircle() | D2 |

#### Canvas 扩展 (7 API, Demo2+3)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 26 | nativeCanvasDrawRoundRect | drawRoundRect() | OH_Drawing_CanvasDrawRoundRect() | D2 |
| 27 | nativeCanvasDrawOval | drawOval() | OH_Drawing_CanvasDrawOval() | D3 |
| 28 | nativeCanvasDrawArc | drawArc() | OH_Drawing_CanvasDrawArc() | D3 |
| 29 | nativeCanvasDrawPath | drawPath(path) | OH_Drawing_CanvasDrawPath() | D3 |
| 30 | nativeCanvasDrawBitmap | drawBitmap(bmp,x,y) | OH_Drawing_CanvasDrawBitmap() | D3 |
| 31 | nativeCanvasDrawImage | drawBitmap(bmp,src,dst) | OH_Drawing_CanvasDrawImageRect() | D3 |
| 32 | nativeCanvasConcat | concat(matrix) | OH_Drawing_CanvasConcat() | 阶段2 |

#### Pen/Brush (10 API)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 33 | nativePenCreate | new Paint(STROKE) | OH_Drawing_PenCreate() | D1 |
| 34 | nativePenSetColor | setColor(color) | OH_Drawing_PenSetColor() | D1 |
| 35 | nativePenSetWidth | setStrokeWidth(w) | OH_Drawing_PenSetWidth() | D1 |
| 36 | nativePenSetAntiAlias | setAntiAlias(true) | OH_Drawing_PenSetAntiAlias() | 阶段2 |
| 37 | nativePenSetCap | setStrokeCap(cap) | OH_Drawing_PenSetCap() | 阶段2 |
| 38 | nativeBrushCreate | new Paint(FILL) | OH_Drawing_BrushCreate() | D1 |
| 39 | nativeBrushSetColor | setColor(color) | OH_Drawing_BrushSetColor() | D1 |
| 40 | nativeBrushSetAntiAlias | setAntiAlias(true) | OH_Drawing_BrushSetAntiAlias() | 阶段2 |
| 41 | nativeBrushSetAlpha | setAlpha(a) | OH_Drawing_BrushSetAlpha() | 阶段2 |
| 42 | nativeBrushSetShader | setShader(shader) | OH_Drawing_BrushSetShaderEffect() | 阶段2 |

#### Path (5 API)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 43 | nativePathCreate | new Path() | OH_Drawing_PathCreate() | 阶段2 |
| 44 | nativePathMoveTo | moveTo(x,y) | OH_Drawing_PathMoveTo() | 阶段2 |
| 45 | nativePathLineTo | lineTo(x,y) | OH_Drawing_PathLineTo() | 阶段2 |
| 46 | nativePathClose | close() | OH_Drawing_PathClose() | 阶段2 |
| 47 | nativePathAddArc | addArc(oval,start,sweep) | OH_Drawing_PathAddArc() | 阶段2 |

#### Bitmap (5 API)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 48 | nativeBitmapCreate | Bitmap.createBitmap(w,h) | OH_Drawing_BitmapCreate() | D1 |
| 49 | nativeBitmapGetPixels | getPixels() | OH_Drawing_BitmapGetPixels() | 阶段2 |
| 50 | nativeBitmapSetPixels | setPixels() | OH_Drawing_BitmapSetPixels() | 阶段2 |
| 51 | nativeBitmapRecycle | recycle() | OH_Drawing_BitmapDestroy() | 阶段2 |
| 52 | nativeBitmapDecode | BitmapFactory.decode() | OH_ImageSource_Create+Decode | 阶段2 |

#### Font (5 API)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 53 | nativeFontCreate | new Typeface() | OH_Drawing_FontCreate() | D1 |
| 54 | nativeFontSetSize | setTextSize(sp) | OH_Drawing_FontSetSize() | D1 |
| 55 | nativeFontMeasureText | measureText(str) | OH_Drawing_FontMeasureText() | D1 |
| 56 | nativeFontGetMetrics | getFontMetrics() | OH_Drawing_FontGetMetrics() | D2 |
| 57 | nativeFontSetTypeface | setTypeface(tf) | OH_Drawing_FontSetTypeface() | 阶段2 |

#### Node (3 API)

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 58 | nativeNodeCreate | ViewRootImpl.setView() | OH_ArkUI_NodeCreate() | 阶段2 |
| 59 | nativeNodeAddChild | ViewGroup.addView() | OH_ArkUI_NodeAddChild() | 阶段2 |
| 60 | nativeNodeSetSize | View.setLayoutParams() | OH_ArkUI_NodeSetSize() | 阶段2 |

**参照**: [H48 Android View↔Compose], [H21 RN Bridge], WINE §原则1-2, Y-skills/A2OH-JAVA-TO-ARKTS.md

---

### 05-Input (3 API) — P1 | 阶段1 Demo2

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 61 | nativeDispatchTouch | dispatchTouchEvent() | OH_ArkUI_NodeEvent(TOUCH) | D2 |
| 62 | nativeDispatchKey | dispatchKeyEvent() | OH_ArkUI_NodeEvent(KEY) | D2 |
| 63 | nativeGetTouchXY | MotionEvent.getX/Y() | OH_ArkUI_PointerEvent_GetX/Y() | D2 |

**参照**: Y-bridge/ Input方法, Y-skills/A2OH-LIFECYCLE.md §事件分发

---

### 06-Preferences (15 API) — P2 | 阶段1 Demo3 + 阶段2

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 64 | nativePrefsOpen | getSharedPreferences(name) | OH_Preferences_Open(name) | D3 |
| 65 | nativePrefsPutString | putString(k,v) | OH_Preferences_PutString(k,v) | D3 |
| 66 | nativePrefsGetString | getString(k,def) | OH_Preferences_GetString(k,def) | D3 |
| 67 | nativePrefsPutInt | putInt(k,v) | OH_Preferences_PutInt(k,v) | D3 |
| 68 | nativePrefsGetInt | getInt(k,def) | OH_Preferences_GetInt(k,def) | D3 |
| 69 | nativePrefsPutBool | putBoolean(k,v) | OH_Preferences_PutBool(k,v) | 阶段2 |
| 70 | nativePrefsGetBool | getBoolean(k,def) | OH_Preferences_GetBool(k,def) | 阶段2 |
| 71 | nativePrefsPutFloat | putFloat(k,v) | OH_Preferences_PutFloat(k,v) | 阶段2 |
| 72 | nativePrefsGetFloat | getFloat(k,def) | OH_Preferences_GetFloat(k,def) | 阶段2 |
| 73 | nativePrefsPutLong | putLong(k,v) | OH_Preferences_PutLong(k,v) | 阶段2 |
| 74 | nativePrefsGetLong | getLong(k,def) | OH_Preferences_GetLong(k,def) | 阶段2 |
| 75 | nativePrefsRemove | remove(key) | OH_Preferences_Delete(key) | 阶段2 |
| 76 | nativePrefsClear | clear() | OH_Preferences_Clear() | 阶段2 |
| 77 | nativePrefsFlush | apply() / commit() | OH_Preferences_Flush() | D3 |
| 78 | nativePrefsClose | N/A (GC) | OH_Preferences_Close() | 阶段2 |

**参照**: Y-skills/A2OH-DATA-LAYER.md, Y-bridge/ Preferences方法, [H06 Mono行为一致性]

---

### 07-Database (20 API) — P2 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 79 | nativeDbOpen | SQLiteDatabase.openOrCreate() | OH_Rdb_GetOrOpen() | B2 |
| 80 | nativeDbClose | close() | OH_Rdb_CloseStore() | B2 |
| 81 | nativeDbExecSQL | execSQL(sql) | OH_Rdb_Execute() | B2 |
| 82 | nativeDbQuery | rawQuery(sql,args) | OH_Rdb_Query() | B2 |
| 83 | nativeDbInsert | insert(table,values) | OH_Rdb_Insert() | B2 |
| 84 | nativeDbUpdate | update(table,values,where) | OH_Rdb_Update() | B2 |
| 85 | nativeDbDelete | delete(table,where) | OH_Rdb_Delete() | B2 |
| 86 | nativeDbBeginTx | beginTransaction() | OH_Rdb_BeginTransaction() | B2 |
| 87 | nativeDbCommitTx | setTransactionSuccessful()+endTransaction() | OH_Rdb_Commit() | B2 |
| 88 | nativeDbRollbackTx | endTransaction() (without success) | OH_Rdb_RollBack() | B2 |
| 89 | nativeCursorNext | cursor.moveToNext() | OH_Cursor_GoToNextRow() | B2 |
| 90 | nativeCursorGetString | cursor.getString(col) | OH_Cursor_GetString() | B2 |
| 91 | nativeCursorGetInt | cursor.getInt(col) | OH_Cursor_GetInt64() | B2 |
| 92 | nativeCursorGetDouble | cursor.getDouble(col) | OH_Cursor_GetReal() | B2 |
| 93 | nativeCursorGetBlob | cursor.getBlob(col) | OH_Cursor_GetBlob() | B2 |
| 94 | nativeCursorClose | cursor.close() | OH_Cursor_Destroy() | B2 |
| 95 | nativeCursorGetCount | cursor.getCount() | OH_Cursor_GetRowCount() | B2 |
| 96 | nativeCursorGetColumnName | cursor.getColumnName(i) | OH_Cursor_GetColumnName() | B2 |
| 97 | nativeCursorGetColumnCount | cursor.getColumnCount() | OH_Cursor_GetColumnCount() | B2 |
| 98 | nativeCursorMoveTo | cursor.moveToPosition(pos) | OH_Cursor_GoToRow() | B2 |

**参照**: Y-skills/A2OH-DATA-LAYER.md §SQLite→RdbStore, [H44 SQLite→IndexedDB], Y-db (api_compat.db查询映射)

---

### 08-Network (3 API) — P2 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 99 | nativeHttpGet | HttpURLConnection.GET | OH_HttpClient_Request(GET) | B2 |
| 100 | nativeHttpPost | HttpURLConnection.POST | OH_HttpClient_Request(POST) | B2 |
| 101 | nativeHttpGetStatus | getResponseCode() | OH_HttpClient_GetStatus() | B2 |

**参照**: Y-skills/A2OH-NETWORKING.md

---

### 09-Notification (5 API) — P3 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 102 | nativeNotifyShow | NotificationManager.notify() | OH_NotificationRequest_Create+Publish | B3 |
| 103 | nativeNotifyCancel | cancel(id) | OH_Notification_Cancel() | B3 |
| 104 | nativeNotifySetTitle | Notification.Builder.setTitle() | OH_NotificationContent_SetTitle() | B3 |
| 105 | nativeNotifySetText | setContentText() | OH_NotificationContent_SetText() | B3 |
| 106 | nativeNotifySetIcon | setSmallIcon() | OH_NotificationContent_SetIcon() | B3 |

**参照**: Y-bridge/ Notification方法, Y-skills/

---

### 10-Media (19 API) — P3 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 107 | nativePlayerCreate | new MediaPlayer() | OH_AVPlayer_Create() | B3 |
| 108 | nativePlayerSetSource | setDataSource(path) | OH_AVPlayer_SetSource() | B3 |
| 109 | nativePlayerPrepare | prepare() | OH_AVPlayer_Prepare() | B3 |
| 110 | nativePlayerStart | start() | OH_AVPlayer_Play() | B3 |
| 111 | nativePlayerPause | pause() | OH_AVPlayer_Pause() | B3 |
| 112 | nativePlayerStop | stop() | OH_AVPlayer_Stop() | B3 |
| 113 | nativePlayerRelease | release() | OH_AVPlayer_Release() | B3 |
| 114 | nativePlayerSeekTo | seekTo(ms) | OH_AVPlayer_Seek() | B3 |
| 115 | nativePlayerGetDuration | getDuration() | OH_AVPlayer_GetDuration() | B3 |
| 116 | nativePlayerGetPosition | getCurrentPosition() | OH_AVPlayer_GetCurrentPosition() | B3 |
| 117 | nativePlayerSetVolume | setVolume(l,r) | OH_AVPlayer_SetVolume() | B3 |
| 118 | nativePlayerIsPlaying | isPlaying() | OH_AVPlayer_IsPlaying() | B3 |
| 119 | nativePlayerSetLoop | setLooping(bool) | OH_AVPlayer_SetLooping() | B3 |
| 120 | nativeAudioCreate | new AudioTrack() | OH_AudioStreamBuilder_Create() | B3 |
| 121 | nativeAudioWrite | write(buffer) | OH_AudioRenderer_Write() | B3 |
| 122 | nativeAudioStart | play() | OH_AudioRenderer_Start() | B3 |
| 123 | nativeAudioStop | stop() | OH_AudioRenderer_Stop() | B3 |
| 124 | nativeAudioRelease | release() | OH_AudioRenderer_Release() | B3 |
| 125 | nativeAudioGetState | getPlayState() | OH_AudioRenderer_GetState() | B3 |

**参照**: Y-skills/A2OH-INDEX.md §Media

---

### 11-DeviceInfo (10 API) — P3 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 126 | nativeGetDeviceName | Build.DEVICE | OH_DeviceInfo_GetDeviceName() | B4 |
| 127 | nativeGetModel | Build.MODEL | OH_DeviceInfo_GetProductModel() | B4 |
| 128 | nativeGetBrand | Build.BRAND | OH_DeviceInfo_GetBrand() | B4 |
| 129 | nativeGetOSVersion | Build.VERSION.RELEASE | OH_DeviceInfo_GetOSVersion() | B4 |
| 130 | nativeGetSDKInt | Build.VERSION.SDK_INT | OH_DeviceInfo_GetSDKApiVersion() | B4 |
| 131 | nativeGetScreenWidth | DisplayMetrics.widthPixels | OH_Display_GetWidth() | B4 |
| 132 | nativeGetScreenHeight | DisplayMetrics.heightPixels | OH_Display_GetHeight() | B4 |
| 133 | nativeGetDensity | DisplayMetrics.density | OH_Display_GetDensity() | B4 |
| 134 | nativeGetLanguage | Locale.getDefault().getLanguage() | OH_Intl_GetLanguage() | B4 |
| 135 | nativeGetCountry | Locale.getDefault().getCountry() | OH_Intl_GetCountry() | B4 |

**参照**: Y-skills/A2OH-DEVICE-API.md, 纯字符串返回 → string_bridge 轻量应用

---

### 12-Sensor (4 API) — P3 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 136 | nativeSensorRegister | registerListener(ACCELEROMETER) | OH_Sensor_Subscribe(ACCELEROMETER) | B4 |
| 137 | nativeSensorUnregister | unregisterListener() | OH_Sensor_Unsubscribe() | B4 |
| 138 | nativeLocationGetLast | getLastKnownLocation() | OH_Location_GetLastLocation() | B4 |
| 139 | nativeLocationRequest | requestLocationUpdates() | OH_Location_StartLocating() | B4 |

**参照**: Y-skills/A2OH-DEVICE-API.md §传感器

---

### 13-Clipboard (5 API) — P3 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 140 | nativeClipboardSet | setPrimaryClip(ClipData.newPlain()) | OH_Pasteboard_SetText() | B4 |
| 141 | nativeClipboardGet | getPrimaryClip().getItemAt(0) | OH_Pasteboard_GetText() | B4 |
| 142 | nativeClipboardHas | hasPrimaryClip() | OH_Pasteboard_HasData() | B4 |
| 143 | nativeVibrate | Vibrator.vibrate(ms) | OH_Vibrator_StartOnce(ms) | B4 |
| 144 | nativeVibrateCancel | Vibrator.cancel() | OH_Vibrator_Stop() | B4 |

---

### 14-ArkUI (14 API) — P0 | 阶段1 部分 + 阶段2

| # | JNI 方法 | Android API | OH API | Demo |
|---|---------|------------|--------|------|
| 145 | nativeArkUINodeCreate | ViewRootImpl初始化 | OH_ArkUI_NodeCreate(type) | D1 |
| 146 | nativeArkUINodeDestroy | View.removeAllViews() | OH_ArkUI_NodeDestroy() | 阶段2 |
| 147 | nativeArkUINodeAddChild | ViewGroup.addView() | OH_ArkUI_NodeAddChild() | D3 |
| 148 | nativeArkUINodeRemoveChild | ViewGroup.removeView() | OH_ArkUI_NodeRemoveChild() | 阶段2 |
| 149 | nativeArkUINodeSetPosition | View.setX/setY() | OH_ArkUI_NodeSetPosition() | 阶段2 |
| 150 | nativeArkUINodeSetSize | View.setLayoutParams() | OH_ArkUI_NodeSetSize() | 阶段2 |
| 151 | nativeArkUINodeSetBgColor | View.setBackgroundColor() | OH_ArkUI_NodeSetBackgroundColor() | 阶段2 |
| 152 | nativeArkUINodeSetOpacity | View.setAlpha() | OH_ArkUI_NodeSetOpacity() | 阶段2 |
| 153 | nativeArkUINodeSetVisible | View.setVisibility() | OH_ArkUI_NodeSetVisibility() | 阶段2 |
| 154 | nativeArkUINodeSetText | TextView.setText() | OH_ArkUI_TextNodeSetContent() | D3 |
| 155 | nativeArkUINodeSetFontSize | TextView.setTextSize() | OH_ArkUI_TextNodeSetFontSize() | 阶段2 |
| 156 | nativeArkUINodeSetImage | ImageView.setImageBitmap() | OH_ArkUI_ImageNodeSetSrc() | 阶段2 |
| 157 | nativeArkUINodeSetOnClick | View.setOnClickListener() | OH_ArkUI_NodeAddEventReceiver(CLICK) | D3 |
| 158 | nativeArkUINodeSetOnTouch | View.setOnTouchListener() | OH_ArkUI_NodeAddEventReceiver(TOUCH) | 阶段2 |

**参照**: [H48 View↔Compose], [H51 Android View→ArkUI], Y-skills/A2OH-JAVA-TO-ARKTS.md

---

### 15-Package (0 直接API, 通过 AMS) — P0 | 阶段1

> 包管理通过 OH 的 BundleManagerService 补丁实现，不需要独立 JNI 方法。
> APK 安装/查询通过 OH BMS 补丁 + appspawn-x 实现。

---

### 其他 (权限+定时器等, 6 API) — P2 | 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 159 | nativeRequestPermission | requestPermissions() | OH_AccessToken_RequestPermission() | B5 |
| 160 | nativePermissionResult | onRequestPermissionsResult() | OH_AccessToken_OnResult() | B5 |
| 161 | nativeAlarmSet | AlarmManager.set() | OH_Reminder_AddTimer() | B5 |
| 162 | nativeAlarmCancel | AlarmManager.cancel() | OH_Reminder_CancelTimer() | B5 |
| 163 | nativeHandlerPost | Handler.post(Runnable) | OH_EventHandler_PostTask() | B5 |
| 164 | nativeHandlerPostDelayed | Handler.postDelayed(r,ms) | OH_EventHandler_PostDelayedTask() | B5 |

---

### Rendering 长尾 (15 API, 凑满 179) — 阶段2

| # | JNI 方法 | Android API | OH API | 批次 |
|---|---------|------------|--------|------|
| 165 | nativeCanvasDrawPoints | drawPoints() | OH_Drawing_CanvasDrawPoints() | B5 |
| 166 | nativeCanvasDrawTextOnPath | drawTextOnPath() | OH_Drawing_CanvasDrawTextOnPath() | B5 |
| 167 | nativePathAddRect | addRect() | OH_Drawing_PathAddRect() | B5 |
| 168 | nativePathAddCircle | addCircle() | OH_Drawing_PathAddCircle() | B5 |
| 169 | nativePathTransform | transform(matrix) | OH_Drawing_PathTransform() | B5 |
| 170 | nativeMatrixCreate | new Matrix() | OH_Drawing_MatrixCreate() | B6 |
| 171 | nativeMatrixSetRotate | setRotate(degrees) | OH_Drawing_MatrixSetRotation() | B6 |
| 172 | nativeMatrixSetScale | setScale(sx,sy) | OH_Drawing_MatrixSetScale() | B6 |
| 173 | nativeMatrixSetTranslate | setTranslate(dx,dy) | OH_Drawing_MatrixSetTranslation() | B6 |
| 174 | nativePenSetStyle | setStyle(STROKE) | OH_Drawing_PenSetJoin() | B6 |
| 175 | nativeBrushSetStyle | setStyle(FILL_AND_STROKE) | OH_Drawing_BrushSetBlendMode() | B6 |
| 176 | nativeShadowLayer | setShadowLayer() | OH_Drawing_FilterSetShadow() | B6 |
| 177 | nativeColorFilter | setColorFilter() | OH_Drawing_FilterSetColorFilter() | B6 |
| 178 | nativeGradientLinear | LinearGradient() | OH_Drawing_ShaderEffectCreateLinearGradient() | B6 |
| 179 | nativeGradientRadial | RadialGradient() | OH_Drawing_ShaderEffectCreateRadialGradient() | B6 |

---

## 阶段 2: AI 全自动铺开 134 API

### 执行模式: 经过 3 Demo 验证的 AI 协作模式

```
对每个剩余 API:
  1. AI 读取本文档的映射规格 (2 min)
  2. AI 读取 Y-db api_compat.db 查映射评分 (1 min)
  3. AI 生成 Mock 实现 — 参照 Y-test/mock/ (3 min)
  4. AI 生成 Bridge 实现 — 参照 str.h RAII + WINE §原则1 (10 min)
  5. Ralph Loop 编译 — 参照 [B11] (5 min, 含自愈)
  6. AI 生成测试 + 运行 — 参照 [B06] (5 min)
  7. API 行为对比 vs 双机基线 — 参照 [H35 wine-tests] (3 min)
  8. LLM-as-Judge 审查 — 参照 [D05] (2 min)
  9. 集成到 .so + 回归 (5 min)

  单个 API 平均: ~35 min
  Swarm 3 路并行 (GZ02 + GZ05 + PC): ~12 min/API
```

### 批次执行计划

| 批次 | API 组 | 数量 | 预计耗时 | 产出 |
|------|--------|------|---------|------|
| **B1** | 剩余 Preferences(10) + Input 细化(1) | 11 | ~2h | 完善 Demo 3 |
| **B2** | Database(20) + Network(3) | 23 | ~5h | 应用数据能力 |
| **B3** | Notification(5) + Media(19) | 24 | ~5h | 系统多媒体能力 |
| **B4** | DeviceInfo(10) + Sensor(4) + Clipboard(5) | 19 | ~4h | 硬件抽象层 |
| **B5** | ArkUI 扩展(8) + 权限(4) + Rendering(5) | 17 | ~3h | 高级 UI + 权限 |
| **B6** | Rendering 长尾(10) + 边界兼容 | 10 | ~2h | 兼容性收尾 |
| **Buffer** | 自愈修复 + 边界条件 | ~30 | ~6h | 质量加固 |
| **合计** | | **134** | **~27h** | |

### 质量门控: 每批次完成后

```
1. 全量 Mock 回归 (所有已完成 API) — [H35 Wine回归套件]
2. 3 Demo E2E 回归 (HelloWorld + Calculator + MockDonalds)
3. 截图 SSIM 对比 > 0.85 — [E08]
4. Confidence 全项 > 0.85 — [D07]
5. 真机增量验证: hdc push .so → kill进程 → 测试 (免ROM!)
6. API 覆盖率更新: 已完成/179 = XX%
```

### 里程碑

| 里程碑 | API 覆盖 | 对应能力 |
|--------|---------|---------|
| 3 Demo 完成 | 45/179 (25%) | 核心链路验证 |
| B1+B2 完成 | 79/179 (44%) | 数据+网络能力 |
| B3 完成 | 103/179 (58%) | 多媒体能力 |
| B4 完成 | 122/179 (68%) | 硬件抽象 |
| B5+B6 完成 | 149/179 (83%) | 高级 UI + 权限 |
| Buffer 完成 | **179/179 (100%)** | **全覆盖** |

---

## 阶段 3: GitHub 自治 (179 API 完成后)

### 开源后的自动进化模式

```
用户提 Issue: "API #142 nativeVibrate 在 Pixel 5 APK 上时长不对"
  ↓ (秒级)
AI Bot 自动认领
  ↓ (3 分钟)
AI 分析: 查看 13-Clipboard/src/ 的 nativeVibrate 实现
AI 发现: OH_Vibrator_StartOnce() 参数单位是 ms，但传入的是 μs
  ↓ (10 分钟)
AI 生成修复 + 测试 + PR
  ↓ (CI 通过)
自动合并 + 自动 Release v1.x.x
  ↓
用户拉取新版 .so → 问题解决

全程零人工。
```

### 兼容性数据库 (Wine AppDB 模式)

```yaml
# 随 API 覆盖率提升，APK 兼容性自动提升
api_coverage: 179/179
tested_apks:
  - name: HelloWorld          rating: platinum   apis: 25
  - name: Calculator          rating: platinum   apis: 33
  - name: MockDonalds         rating: gold       apis: 45
  - name: WeChat (basic)      rating: silver     apis: 89
  - name: Alipay (basic)      rating: bronze     apis: 120
```

---

## 给领导的核心叙事

```
第 1 周:  3 Demo 验证 AI 自治研发模式 (45 API, 25%)
第 2-3 周: AI 全自动铺开剩余 API   (134 API, 75%)  ← 不需要加人!
第 4 周:  GitHub 开源，社区驱动    (179 API, 100%)

关键对比:
传统方案: 20人 × 6个月 = 120人月
我们:     1人 × 1个月 = 1人月 + AI
效率提升: 120倍
```
