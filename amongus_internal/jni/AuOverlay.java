package au.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Toast;

/**
 * Custom-drawn Among Us internal menu + ESP canvas.
 */
public class AuOverlay {
    private static Activity activity;
    private static WindowManager wm;
    private static WindowManager.LayoutParams fabLp;
    private static WindowManager.LayoutParams panelLp;
    private static WindowManager.LayoutParams espLp;
    private static FabView fab;
    private static MenuView panel;
    private static EspView espView;
    private static boolean menuOpen = false;
    private static boolean attached = false;
    private static int tries = 0;
    private static final Handler ui = new Handler(Looper.getMainLooper());

    private static final Runnable ticker = new Runnable() {
        @Override public void run() {
            if (espView != null) espView.invalidate();
            if (panel != null && menuOpen) panel.invalidate();
            if (attached) ui.postDelayed(this, 16);
        }
    };

    private static final Runnable attachRetry = new Runnable() {
        @Override public void run() { tryAttach(); }
    };

    public static native int nativeEspCount();
    /** @return number of ESP entries written into buf (stride 10) */
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

    public static void start(final Activity act) {
        activity = act;
        ui.post(new Runnable() {
            @Override public void run() {
                nativeLog("AuOverlay.start UI");
                tryAttach();
            }
        });
    }

    private static void tryAttach() {
        if (attached || activity == null) return;
        tries++;
        try {
            View decor = activity.getWindow().getDecorView();
            if (decor.getWindowToken() == null && tries < 50) {
                decor.postDelayed(attachRetry, 200);
                return;
            }
            attachNow();
        } catch (Throwable t) {
            nativeLog("tryAttach: " + t);
            if (tries < 50) ui.postDelayed(attachRetry, 300);
        }
    }

    private static void attachNow() {
        if (attached) return;
        Context ctx = activity;
        wm = activity.getWindowManager();

        espView = new EspView(ctx);
        espLp = params(WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT, false);
        espLp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        espLp.gravity = Gravity.TOP | Gravity.START;
        wm.addView(espView, espLp);

        fab = new FabView(ctx);
        fabLp = params(dp(72), dp(72), true);
        fabLp.gravity = Gravity.START | Gravity.CENTER_VERTICAL;
        fabLp.x = dp(14);
        fabLp.y = 0;
        wm.addView(fab, fabLp);

        panel = new MenuView(ctx);
        panel.setVisibility(View.GONE);
        panelLp = params(dp(340), dp(460), true);
        panelLp.gravity = Gravity.CENTER;
        wm.addView(panel, panelLp);

        fab.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                menuOpen = !menuOpen;
                panel.setVisibility(menuOpen ? View.VISIBLE : View.GONE);
                fab.setOpen(menuOpen);
                try { wm.updateViewLayout(panel, panelLp); } catch (Throwable ignored) {}
            }
        });

        attached = true;
        ui.post(ticker);
        try {
            Toast.makeText(activity, "AU ready", Toast.LENGTH_SHORT).show();
        } catch (Throwable ignored) {}
        nativeLog("ATTACH OK");
    }

    private static WindowManager.LayoutParams params(int w, int h, boolean touchable) {
        int type = WindowManager.LayoutParams.TYPE_APPLICATION;
        try {
            if (Build.VERSION.SDK_INT >= 23 && Settings.canDrawOverlays(activity))
                type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
        } catch (Throwable ignored) {}

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

    // ---------------- FAB ----------------
    public static class FabView extends View {
        private final Paint bg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint ring = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint tx = new Paint(Paint.ANTI_ALIAS_FLAG);
        private boolean open;

        public FabView(Context ctx) {
            super(ctx);
            bg.setStyle(Paint.Style.FILL);
            ring.setStyle(Paint.Style.STROKE);
            ring.setStrokeWidth(dp(2));
            tx.setColor(Color.WHITE);
            tx.setTextAlign(Paint.Align.CENTER);
            tx.setTypeface(Typeface.create(Typeface.SANS_SERIF, Typeface.BOLD));
            tx.setTextSize(dp(13));
        }

        void setOpen(boolean v) { open = v; invalidate(); }

        @Override protected void onDraw(Canvas c) {
            float cx = getWidth() * 0.5f, cy = getHeight() * 0.5f, r = getWidth() * 0.42f;
            bg.setColor(open ? 0xE0FF5252 : 0xE0E53935);
            c.drawCircle(cx, cy, r, bg);
            ring.setColor(0x66FFFFFF);
            c.drawCircle(cx, cy, r, ring);
            c.drawText(open ? "CLOSE" : "MENU", cx, cy + dp(5), tx);
        }
    }

    // ---------------- MENU ----------------
    public static class MenuView extends View {
        private final Paint panelBg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint accent = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint title = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint sub = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint rowBg = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint rowTx = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint track = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint thumb = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint footer = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final RectF tmp = new RectF();
        private final String[] labels = {
                "Player ESP", "Murder ESP", "Boxes", "Snaplines", "Names / Roles"
        };
        private final boolean[] values = new boolean[5];

        public MenuView(Context ctx) {
            super(ctx);
            title.setColor(Color.WHITE);
            title.setTextSize(dp(20));
            title.setTypeface(Typeface.create("sans-serif-medium", Typeface.NORMAL));
            sub.setColor(0xFF9AA3B2);
            sub.setTextSize(dp(12));
            rowTx.setColor(0xFFE8ECF1);
            rowTx.setTextSize(dp(15));
            footer.setColor(0xFF7A8494);
            footer.setTextSize(dp(11));
            syncFromNative();
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
            invalidate();
        }

        @Override protected void onDraw(Canvas c) {
            float w = getWidth(), h = getHeight();
            float rad = dp(22);
            tmp.set(0, 0, w, h);
            panelBg.setShader(new LinearGradient(0, 0, w, h,
                    0xF0161B22, 0xF00E1218, Shader.TileMode.CLAMP));
            c.drawRoundRect(tmp, rad, rad, panelBg);

            // top accent bar
            accent.setColor(0xFFE53935);
            tmp.set(0, 0, w, dp(4));
            c.drawRoundRect(tmp, rad, rad, accent);
            tmp.set(0, dp(2), w, dp(4));
            c.drawRect(tmp, accent);

            // brand
            c.drawText("AMONG US", dp(22), dp(40), title);
            accent.setColor(0xFFFF6B6B);
            c.drawText("INTERNAL", dp(22) + title.measureText("AMONG US "), dp(40), accent);
            c.drawText("2026.6.5  ·  arm64  ·  Kitty", dp(22), dp(58), sub);

            float y = dp(78);
            for (int i = 0; i < labels.length; i++) {
                float rowH = dp(52);
                tmp.set(dp(14), y, w - dp(14), y + rowH);
                rowBg.setColor(i % 2 == 0 ? 0x14FFFFFF : 0x0AFFFFFF);
                c.drawRoundRect(tmp, dp(14), dp(14), rowBg);
                c.drawText(labels[i], dp(28), y + dp(32), rowTx);
                drawSwitch(c, w - dp(28), y + rowH * 0.5f, values[i]);
                y += rowH + dp(8);
            }

            int pc = 0, mc = 0;
            try { pc = nativePlayerCount(); mc = nativeMurderCount(); } catch (Throwable ignored) {}
            c.drawText("Players " + pc + "   ·   Murder " + mc, dp(22), h - dp(22), footer);
        }

        private void drawSwitch(Canvas c, float right, float cy, boolean on) {
            float tw = dp(46), th = dp(26);
            float left = right - tw;
            tmp.set(left, cy - th * 0.5f, right, cy + th * 0.5f);
            track.setColor(on ? 0xFFE53935 : 0xFF2A3140);
            c.drawRoundRect(tmp, th, th, track);
            float thumbR = th * 0.38f;
            float tx = on ? (right - th * 0.5f) : (left + th * 0.5f);
            thumb.setColor(Color.WHITE);
            c.drawCircle(tx, cy, thumbR, thumb);
        }

        @Override public boolean onTouchEvent(MotionEvent e) {
            if (e.getAction() != MotionEvent.ACTION_UP) return true;
            float y = dp(78);
            float rowH = dp(52);
            float gap = dp(8);
            for (int i = 0; i < labels.length; i++) {
                if (e.getY() >= y && e.getY() <= y + rowH) {
                    apply(i, !values[i]);
                    return true;
                }
                y += rowH + gap;
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
        private final float[] buf = new float[16 * 10]; // sx,top,bot,r,g,b,a,flags, _pad
        private final RectF rr = new RectF();

        public EspView(Context ctx) {
            super(ctx);
            setWillNotDraw(false);
            setBackgroundColor(Color.TRANSPARENT);
            box.setStyle(Paint.Style.STROKE);
            line.setStyle(Paint.Style.STROKE);
            line.setStrokeWidth(2.2f);
            text.setTextSize(dp(13));
            text.setTypeface(Typeface.create(Typeface.SANS_SERIF, Typeface.BOLD));
            text.setShadowLayer(3f, 0, 1f, 0xCC000000);
            textBg.setColor(0x99000000);
        }

        @Override protected void onSizeChanged(int w, int h, int oW, int oH) {
            super.onSizeChanged(w, h, oW, oH);
            try { nativeSetViewSize(w, h); } catch (Throwable ignored) {}
        }

        @Override protected void onDraw(Canvas canvas) {
            // Use fill return count (count()+fill() raced / soft-cull desync).
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

                float hw = Math.max(dp(16), (bot - top) * 0.38f);
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
                    float len = Math.max(dp(8), (bot - top) * 0.22f);
                    // corner brackets
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
                            float ty = top - dp(8);
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
