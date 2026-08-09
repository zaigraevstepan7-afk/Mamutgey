package au.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.RadialGradient;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.DecelerateInterpolator;

/**
 * Compact floating menu + small white bottom handle. Menu is draggable.
 */
public class AuOverlay {
    private static Activity activity;
    private static WindowManager wm;
    private static WindowManager.LayoutParams handleLp;
    private static WindowManager.LayoutParams sheetLp;
    private static WindowManager.LayoutParams espLp;
    private static HandleView handle;
    private static MenuSheet sheet;
    private static EspView espView;
    private static boolean menuOpen = false;
    private static boolean attached = false;
    private static int tries = 0;
    private static final Handler ui = new Handler(Looper.getMainLooper());

    static final int C_PANEL = 0xF20C1018;
    static final int C_PANEL2 = 0xF0141A24;
    static final int C_CRIMSON = 0xFFE11D48;
    static final int C_CRIMSON_S = 0xFFFF4D6D;
    static final int C_ICE = 0xFFE8EEF7;
    static final int C_MUTED = 0xFF8B95A8;
    static final int C_LINE = 0x1AFFFFFF;

    private static final Runnable ticker = new Ticker();
    private static final Runnable attachRetry = new AttachRetry();

    private static class Ticker implements Runnable {
        public void run() {
            if (espView != null) espView.invalidate();
            if (handle != null) handle.invalidate();
            if (sheet != null && (menuOpen || sheet.needsAnim())) sheet.invalidate();
            if (attached) ui.postDelayed(this, 16);
        }
    }

    private static class AttachRetry implements Runnable {
        public void run() { tryAttach(); }
    }

    public static native int nativeEspCount();
    public static native int nativeEspFill(float[] buf);
    public static native String nativeEspLabel(int index);
    public static native void nativeSetEsp(boolean v);
    public static native void nativeSetMurderEsp(boolean v);
    public static native void nativeSetBox(boolean v);
    public static native void nativeSetLine(boolean v);
    public static native void nativeSetName(boolean v);
    public static native boolean nativeGetEsp();
    public static native boolean nativeGetMurderEsp();
    public static native boolean nativeGetBox();
    public static native boolean nativeGetLine();
    public static native boolean nativeGetName();
    public static native void nativeSetViewSize(int w, int h);
    public static native void nativeLog(String msg);
    public static native int nativePlayerCount();
    public static native int nativeMurderCount();

    public static void start(Activity act) {
        activity = act;
        ui.post(new StartUi());
    }

    private static class StartUi implements Runnable {
        public void run() { tryAttach(); }
    }

    private static void tryAttach() {
        if (attached || activity == null) return;
        tries++;
        try {
            View decor = activity.getWindow().getDecorView();
            if (decor.getWindowToken() == null && tries < 80) {
                decor.postDelayed(attachRetry, 200);
                return;
            }
            attachNow();
        } catch (Throwable t) {
            if (tries < 80) ui.postDelayed(attachRetry, 300);
        }
    }

    static void setMenuOpen(boolean open) {
        if (menuOpen == open) return;
        menuOpen = open;
        if (sheet != null) sheet.animateOpen(open);
        if (handle != null) handle.setExpanded(open);
    }

    static void toggleMenu() {
        setMenuOpen(!menuOpen);
    }

    static void moveSheet(int dx, int dy) {
        if (sheetLp == null || wm == null || sheet == null) return;
        sheetLp.x += dx;
        sheetLp.y += dy;
        // keep mostly on-screen
        int sw = activity.getResources().getDisplayMetrics().widthPixels;
        int sh = activity.getResources().getDisplayMetrics().heightPixels;
        int maxX = Math.max(0, sw - sheetLp.width);
        int maxY = Math.max(0, sh - sheetLp.height);
        if (sheetLp.x < 0) sheetLp.x = 0;
        if (sheetLp.y < 0) sheetLp.y = 0;
        if (sheetLp.x > maxX) sheetLp.x = maxX;
        if (sheetLp.y > maxY) sheetLp.y = maxY;
        try { wm.updateViewLayout(sheet, sheetLp); } catch (Throwable ignored) {}
    }

    private static void attachNow() {
        if (attached) return;
        try {
            Context ctx = activity;
            wm = activity.getWindowManager();
            int sw = activity.getResources().getDisplayMetrics().widthPixels;
            int sh = activity.getResources().getDisplayMetrics().heightPixels;

            espView = new EspView(ctx);
            espLp = params(WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.MATCH_PARENT, false);
            espLp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
            espLp.gravity = Gravity.TOP | Gravity.START;
            wm.addView(espView, espLp);

            // Compact floating panel (not full-width sheet)
            int panelW = dp(268);
            int panelH = dp(318);
            sheet = new MenuSheet(ctx);
            sheetLp = params(panelW, panelH, true);
            sheetLp.gravity = Gravity.TOP | Gravity.START;
            sheetLp.x = Math.max(0, (sw - panelW) / 2);
            sheetLp.y = Math.max(0, (sh - panelH) / 2 - dp(20));
            sheet.setVisibility(View.GONE);
            sheet.setAlpha(0f);
            wm.addView(sheet, sheetLp);

            // Small white handle — short bar at bottom center
            handle = new HandleView(ctx);
            handleLp = params(dp(72), dp(22), true);
            handleLp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
            handleLp.y = dp(8);
            wm.addView(handle, handleLp);
            handle.setOnClickListener(new HandleClick());

            attached = true;
            ui.post(ticker);
        } catch (Throwable t) {
            attached = false;
            espView = null;
            sheet = null;
            handle = null;
            if (tries < 80) ui.postDelayed(attachRetry, 400);
        }
    }

    private static class HideSheet implements Runnable {
        public void run() {
            if (sheet != null) sheet.finishHide();
        }
    }

    private static class HandleClick implements View.OnClickListener {
        public void onClick(View v) { toggleMenu(); }
    }

    private static WindowManager.LayoutParams params(int w, int h, boolean touchable) {
        int type = WindowManager.LayoutParams.TYPE_APPLICATION;
        int flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
        if (!touchable) flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        else flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;

        WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                w, h, type, flags, PixelFormat.TRANSLUCENT);
        lp.format = PixelFormat.TRANSLUCENT;
        return lp;
    }

    private static int dp(int v) {
        return Math.round(v * activity.getResources().getDisplayMetrics().density);
    }

    static float dpf(float v) {
        return v * activity.getResources().getDisplayMetrics().density;
    }

    // ---------------- SMALL WHITE HANDLE ----------------
    public static class HandleView extends View {
        private final Paint bar = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint glow = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final RectF r = new RectF();
        private boolean expanded;
        private final long t0 = System.nanoTime();

        public HandleView(Context ctx) {
            super(ctx);
            setClickable(true);
            bar.setStyle(Paint.Style.FILL);
            glow.setStyle(Paint.Style.FILL);
        }

        void setExpanded(boolean v) {
            expanded = v;
            invalidate();
        }

        @Override protected void onDraw(Canvas c) {
            float w = getWidth(), h = getHeight();
            float age = (System.nanoTime() - t0) / 1e9f;
            float pulse = 0.5f + 0.5f * (float) Math.sin(age * 2.4);

            float barW = expanded ? dpf(28) : dpf(40);
            float barH = dpf(3.5f);
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            r.set(cx - barW * 0.5f, cy - barH * 0.5f, cx + barW * 0.5f, cy + barH * 0.5f);

            if (!expanded) {
                glow.setColor(Color.argb((int) (40 + 30 * pulse), 255, 255, 255));
                c.drawCircle(cx, cy, dpf(10), glow);
            }

            bar.setColor(0xFFF5F7FA);
            bar.setAlpha(expanded ? 160 : 235);
            c.drawRoundRect(r, barH, barH, bar);
        }
    }

    // ---------------- COMPACT DRAGGABLE MENU ----------------
    public static class MenuSheet extends View {
        private final Paint bg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint accent = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint brand = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint brandSub = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint meta = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint rowLabel = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint rowHint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint divider = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint track = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint thumb = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint thumbSh = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint chipBg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint chipTx = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint chipStroke = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint rowWash = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint wash = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint grip = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final RectF tmp = new RectF();

        private final float[] thumbNow = new float[]{0, 0, 0, 0, 0};
        private final float[] thumbTo = new float[]{0, 0, 0, 0, 0};

        private final String[] labels = {
                "Player ESP", "Murder ESP", "Boxes", "Snaplines", "Names / Roles"
        };
        private final String[] hints = {
                "crew", "impostors", "brackets", "lines", "tags"
        };
        private final boolean[] values = new boolean[5];

        private boolean closing;
        private boolean dragging;
        private float lastRawX, lastRawY;
        private float downX, downY;
        private boolean moved;

        public MenuSheet(Context ctx) {
            super(ctx);
            setClickable(true);

            brand.setColor(C_ICE);
            brand.setTextSize(dpf(18));
            brand.setTypeface(Typeface.create("sans-serif-black", Typeface.NORMAL));

            brandSub.setColor(C_CRIMSON_S);
            brandSub.setTextSize(dpf(9));
            brandSub.setTypeface(Typeface.create("sans-serif-medium", Typeface.BOLD));
            if (Build.VERSION.SDK_INT >= 21) brandSub.setLetterSpacing(0.22f);

            meta.setColor(C_MUTED);
            meta.setTextSize(dpf(9));

            rowLabel.setColor(C_ICE);
            rowLabel.setTextSize(dpf(13));
            rowLabel.setTypeface(Typeface.create("sans-serif-medium", Typeface.NORMAL));

            rowHint.setColor(0xFF6B7588);
            rowHint.setTextSize(dpf(9));

            divider.setColor(C_LINE);
            divider.setStrokeWidth(1f);

            chipTx.setColor(C_ICE);
            chipTx.setTextSize(dpf(10));
            chipTx.setTypeface(Typeface.create("sans-serif-medium", Typeface.NORMAL));
            chipTx.setTextAlign(Paint.Align.CENTER);

            chipStroke.setStyle(Paint.Style.STROKE);
            chipStroke.setStrokeWidth(dpf(1));

            thumbSh.setColor(0x44000000);
            grip.setColor(0x88FFFFFF);
            accent.setStrokeCap(Paint.Cap.ROUND);
            accent.setStrokeWidth(dpf(2));

            syncFromNative();
            for (int i = 0; i < 5; i++) {
                thumbNow[i] = values[i] ? 1f : 0f;
                thumbTo[i] = thumbNow[i];
            }
        }

        boolean needsAnim() {
            for (int i = 0; i < 5; i++) {
                if (Math.abs(thumbNow[i] - thumbTo[i]) > 0.001f) return true;
            }
            return false;
        }

        void animateOpen(boolean open) {
            animate().cancel();
            closing = !open;
            if (open) {
                setVisibility(VISIBLE);
                setScaleX(0.92f);
                setScaleY(0.92f);
                setAlpha(0f);
                animate()
                        .scaleX(1f).scaleY(1f).alpha(1f)
                        .setDuration(200)
                        .setInterpolator(new DecelerateInterpolator())
                        .start();
            } else {
                animate()
                        .scaleX(0.94f).scaleY(0.94f).alpha(0f)
                        .setDuration(160)
                        .setInterpolator(new DecelerateInterpolator())
                        .start();
                ui.postDelayed(new HideSheet(), 170);
            }
        }

        void finishHide() {
            if (closing) {
                setVisibility(GONE);
                setAlpha(0f);
                setScaleX(1f);
                setScaleY(1f);
            }
        }

        private void syncFromNative() {
            try {
                values[0] = nativeGetEsp();
                values[1] = nativeGetMurderEsp();
                values[2] = nativeGetBox();
                values[3] = nativeGetLine();
                values[4] = nativeGetName();
            } catch (Throwable ignored) {}
        }

        private void apply(int i, boolean v) {
            values[i] = v;
            try {
                if (i == 0) nativeSetEsp(v);
                else if (i == 1) nativeSetMurderEsp(v);
                else if (i == 2) nativeSetBox(v);
                else if (i == 3) nativeSetLine(v);
                else if (i == 4) nativeSetName(v);
            } catch (Throwable ignored) {}
            thumbTo[i] = v ? 1f : 0f;
            invalidate();
        }

        private void tickThumbs() {
            for (int i = 0; i < 5; i++) {
                float d = thumbTo[i] - thumbNow[i];
                if (Math.abs(d) < 0.002f) thumbNow[i] = thumbTo[i];
                else thumbNow[i] += d * 0.28f;
            }
        }

        private float headerH() { return dpf(58); }
        private float rowStart() { return dpf(66); }
        private float rowH() { return dpf(40); }

        @Override protected void onDraw(Canvas c) {
            tickThumbs();
            float w = getWidth(), h = getHeight();
            if (w <= 0 || h <= 0) return;
            float rad = dpf(18);

            tmp.set(0, 0, w, h);
            bg.setShader(new LinearGradient(0, 0, 0, h, C_PANEL2, C_PANEL, Shader.TileMode.CLAMP));
            c.drawRoundRect(tmp, rad, rad, bg);

            wash.setShader(new RadialGradient(w * 0.9f, dpf(-10), w * 0.7f,
                    0x44E11D48, Color.TRANSPARENT, Shader.TileMode.CLAMP));
            c.drawRoundRect(tmp, rad, rad, wash);

            accent.setStyle(Paint.Style.FILL);
            accent.setShader(new LinearGradient(0, 0, w, 0,
                    0x00FF4D6D, 0xFFE11D48, Shader.TileMode.CLAMP));
            c.drawRect(rad, 0, w - rad, dpf(2.5f), accent);
            accent.setShader(null);

            // drag grip
            float gw = dpf(28), gh = dpf(3);
            tmp.set(w * 0.5f - gw * 0.5f, dpf(8), w * 0.5f + gw * 0.5f, dpf(8) + gh);
            c.drawRoundRect(tmp, gh, gh, grip);

            float bx = dpf(14);
            c.drawText("AMONG US", bx, dpf(32), brand);
            float brandW = brand.measureText("AMONG US ");
            c.drawText("INTERNAL", bx + brandW, dpf(32), brandSub);
            c.drawText("2026.6.5 · Kitty", bx, dpf(46), meta);

            float y = rowStart();
            float rh = rowH();
            float padX = dpf(12);

            for (int i = 0; i < labels.length; i++) {
                float t = thumbNow[i];
                if (t > 0.01f) {
                    rowWash.setColor(Color.argb((int) (24 * t), 225, 29, 72));
                    tmp.set(padX - dpf(2), y, w - padX + dpf(2), y + rh);
                    c.drawRoundRect(tmp, dpf(10), dpf(10), rowWash);
                }
                c.drawText(labels[i], padX, y + dpf(17), rowLabel);
                c.drawText(hints[i], padX, y + dpf(30), rowHint);
                drawSwitch(c, w - padX, y + rh * 0.5f, t);
                if (i < labels.length - 1) {
                    c.drawLine(padX, y + rh, w - padX, y + rh, divider);
                }
                y += rh;
            }

            int pc = 0, mc = 0;
            try {
                pc = nativePlayerCount();
                mc = nativeMurderCount();
            } catch (Throwable ignored) {}
            float chipY = h - dpf(22);
            drawChip(c, dpf(12), chipY, "P " + pc, false);
            drawChip(c, dpf(12) + dpf(58), chipY, "M " + mc, mc > 0);
        }

        private void drawChip(Canvas c, float x, float cy, String text, boolean hot) {
            float tw = chipTx.measureText(text);
            float ph = dpf(20), pw = tw + dpf(16);
            tmp.set(x, cy - ph * 0.5f, x + pw, cy + ph * 0.5f);
            chipBg.setColor(hot ? 0x33E11D48 : 0x18FFFFFF);
            c.drawRoundRect(tmp, dpf(8), dpf(8), chipBg);
            chipStroke.setColor(hot ? 0x88E11D48 : 0x22FFFFFF);
            c.drawRoundRect(tmp, dpf(8), dpf(8), chipStroke);
            chipTx.setColor(hot ? C_CRIMSON_S : C_ICE);
            c.drawText(text, x + pw * 0.5f, cy + dpf(3.5f), chipTx);
        }

        private void drawSwitch(Canvas c, float right, float cy, float t) {
            float tw = dpf(40), th = dpf(22);
            float left = right - tw;
            tmp.set(left, cy - th * 0.5f, right, cy + th * 0.5f);
            int r = (int) (26 + (225 - 26) * t);
            int g = (int) (34 + (29 - 34) * t);
            int b = (int) (48 + (72 - 48) * t);
            track.setColor(Color.rgb(r, g, b));
            c.drawRoundRect(tmp, th, th, track);
            float thumbR = th * 0.34f;
            float tx = left + th * 0.5f + (tw - th) * t;
            c.drawCircle(tx, cy + dpf(0.8f), thumbR, thumbSh);
            thumb.setColor(0xFFFAFBFD);
            c.drawCircle(tx, cy, thumbR, thumb);
        }

        @Override public boolean onTouchEvent(MotionEvent e) {
            switch (e.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    lastRawX = e.getRawX();
                    lastRawY = e.getRawY();
                    downX = e.getX();
                    downY = e.getY();
                    moved = false;
                    // drag from header / grip zone, or anywhere with long press feel — header preferred
                    dragging = downY <= headerH() + dpf(8);
                    return true;

                case MotionEvent.ACTION_MOVE:
                    float dx = e.getRawX() - lastRawX;
                    float dy = e.getRawY() - lastRawY;
                    if (!dragging) {
                        // allow drag from anywhere if finger moved enough (hold-drag)
                        if (Math.abs(e.getX() - downX) + Math.abs(e.getY() - downY) > dpf(10)) {
                            dragging = true;
                        }
                    }
                    if (dragging && (Math.abs(dx) > 0.5f || Math.abs(dy) > 0.5f)) {
                        moved = true;
                        moveSheet(Math.round(dx), Math.round(dy));
                        lastRawX = e.getRawX();
                        lastRawY = e.getRawY();
                    }
                    return true;

                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    if (!moved) {
                        float y0 = rowStart();
                        float rh = rowH();
                        for (int i = 0; i < labels.length; i++) {
                            float top = y0 + i * rh;
                            if (e.getY() >= top && e.getY() <= top + rh) {
                                apply(i, !values[i]);
                                dragging = false;
                                return true;
                            }
                        }
                    }
                    dragging = false;
                    return true;
            }
            return true;
        }
    }

    // ---------------- ESP ----------------
    public static class EspView extends View {
        private final Paint box = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint line = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint text = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint textBg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final float[] buf = new float[16 * 10];
        private final RectF rr = new RectF();

        public EspView(Context ctx) {
            super(ctx);
            setWillNotDraw(false);
            setBackgroundColor(Color.TRANSPARENT);
            box.setStyle(Paint.Style.STROKE);
            line.setStyle(Paint.Style.STROKE);
            line.setStrokeWidth(2.2f);
            text.setTextSize(dpf(13));
            text.setTypeface(Typeface.create("sans-serif-medium", Typeface.BOLD));
            text.setShadowLayer(3f, 0, 1f, 0xCC000000);
            textBg.setColor(0x99000000);
        }

        @Override protected void onSizeChanged(int w, int h, int oW, int oH) {
            super.onSizeChanged(w, h, oW, oH);
            try { nativeSetViewSize(w, h); } catch (Throwable ignored) {}
        }

        @Override protected void onDraw(Canvas canvas) {
            int n;
            try { n = nativeEspFill(buf); } catch (Throwable t) { return; }
            if (n <= 0) return;
            if (n > 16) n = 16;

            float midX = getWidth() * 0.5f;
            float baseY = getHeight() * 0.88f;

            for (int i = 0; i < n; i++) {
                int o = i * 10;
                float sx = buf[o], top = buf[o + 1], bot = buf[o + 2];
                int col = Color.argb(c(buf[o + 6]), c(buf[o + 3]), c(buf[o + 4]), c(buf[o + 5]));
                int flags = (int) buf[o + 7];
                if (flags == 0) continue;

                boolean drawBox = (flags & 1) != 0;
                boolean drawLine = (flags & 2) != 0;
                boolean murder = (flags & 4) != 0;
                boolean drawName = (flags & 8) != 0;

                float hw = Math.max(dpf(16), (bot - top) * 0.38f);
                float left = sx - hw, right = sx + hw;
                float thick = murder ? 3.6f : 2.4f;

                if (drawLine) {
                    line.setColor(col);
                    line.setAlpha(180);
                    canvas.drawLine(midX, baseY, sx, bot, line);
                }

                if (drawBox) {
                    box.setColor(col);
                    box.setStrokeWidth(thick);
                    float len = Math.max(dpf(8), (bot - top) * 0.22f);
                    canvas.drawLine(left, top, left + len, top, box);
                    canvas.drawLine(left, top, left, top + len, box);
                    canvas.drawLine(right, top, right - len, top, box);
                    canvas.drawLine(right, top, right, top + len, box);
                    canvas.drawLine(left, bot, left + len, bot, box);
                    canvas.drawLine(left, bot, left, bot - len, box);
                    canvas.drawLine(right, bot, right - len, bot, box);
                    canvas.drawLine(right, bot, right, bot - len, box);
                    if (murder) {
                        box.setAlpha(70);
                        box.setStrokeWidth(1.5f);
                        rr.set(left - 3, top - 3, right + 3, bot + 3);
                        canvas.drawRoundRect(rr, 6, 6, box);
                        box.setAlpha(255);
                    }
                }

                if (drawName) {
                    try {
                        String lab = nativeEspLabel(i);
                        if (lab != null && lab.length() > 0) {
                            float tw = text.measureText(lab);
                            float tx = sx - tw * 0.5f;
                            float ty = top - dpf(8);
                            rr.set(tx - 6, ty - text.getTextSize(), tx + tw + 6, ty + 4);
                            canvas.drawRoundRect(rr, 8, 8, textBg);
                            text.setColor(col);
                            canvas.drawText(lab, tx, ty, text);
                        }
                    } catch (Throwable ignored) {}
                }
            }
        }

        private static int c(float v) {
            v *= 255f;
            if (v < 0) return 0;
            if (v > 255) return 255;
            return (int) v;
        }
    }
}
