package au.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * LGL-style Activity WindowManager overlay (TYPE_APPLICATION).
 * Does NOT use content.addView (Unity SurfaceView covers it).
 */
public class AuOverlay {
    private static final String TAG = "AUInternal";
    private static Activity activity;
    private static WindowManager wm;
    private static WindowManager.LayoutParams fabLp;
    private static WindowManager.LayoutParams panelLp;
    private static WindowManager.LayoutParams espLp;
    private static Button fab;
    private static ScrollView panel;
    private static EspView espView;
    private static boolean menuOpen = false;
    private static boolean attached = false;
    private static int tries = 0;
    private static final Handler ui = new Handler(Looper.getMainLooper());

    private static final Runnable ticker = new Runnable() {
        @Override public void run() {
            if (espView != null) espView.invalidate();
            if (attached) ui.postDelayed(this, 16);
        }
    };

    private static final Runnable attachRetry = new Runnable() {
        @Override public void run() {
            tryAttach();
        }
    };

    public static native int nativeEspCount();
    public static native void nativeEspFill(float[] buf);
    public static native String nativeEspLabel(int index);
    public static native void nativeSetEsp(boolean v);
    public static native void nativeSetMurderEsp(boolean v);
    public static native void nativeSetBox(boolean v);
    public static native void nativeSetLine(boolean v);
    public static native void nativeSetName(boolean v);
    public static native boolean nativeGetEsp();
    public static native boolean nativeGetMurderEsp();
    public static native void nativeSetViewSize(int w, int h);
    public static native void nativeLog(String msg);

    public static void start(final Activity act) {
        activity = act;
        ui.post(new Runnable() {
            @Override public void run() {
                nativeLog("AuOverlay.start on UI thread");
                try {
                    Toast.makeText(act, "AU loading menu…", Toast.LENGTH_SHORT).show();
                } catch (Throwable t) {
                    nativeLog("toast1 fail: " + t);
                }
                tryAttach();
            }
        });
    }

    private static void tryAttach() {
        if (attached) return;
        tries++;
        if (activity == null) {
            nativeLog("activity null");
            return;
        }
        try {
            View decor = activity.getWindow().getDecorView();
            if (decor.getWindowToken() == null && tries < 50) {
                nativeLog("window token null, retry " + tries);
                decor.postDelayed(attachRetry, 200);
                return;
            }
            attachNow();
        } catch (Throwable t) {
            nativeLog("tryAttach error: " + t);
            if (tries < 50) ui.postDelayed(attachRetry, 300);
            else {
                try {
                    Toast.makeText(activity, "AU FAIL: " + t.getMessage(), Toast.LENGTH_LONG).show();
                } catch (Throwable ignored) {}
            }
        }
    }

    private static void attachNow() {
        if (attached) return;
        Context ctx = activity;
        wm = activity.getWindowManager();

        // --- ESP layer (not touchable) ---
        espView = new EspView(ctx);
        espLp = params(
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT,
                false);
        espLp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        espLp.gravity = Gravity.TOP | Gravity.START;
        wm.addView(espView, espLp);
        nativeLog("esp view added");

        // --- FAB ---
        fab = new Button(ctx);
        fab.setText("MENU");
        fab.setTextColor(Color.WHITE);
        fab.setTextSize(18f);
        fab.setAllCaps(true);
        fab.setBackgroundColor(0xF0E53935);
        fabLp = params(dp(96), dp(96), true);
        fabLp.gravity = Gravity.START | Gravity.CENTER_VERTICAL;
        fabLp.x = dp(18);
        fabLp.y = 0;
        wm.addView(fab, fabLp);
        nativeLog("fab added");

        // --- Panel ---
        panel = buildPanel(ctx);
        panel.setVisibility(View.GONE);
        panelLp = params(dp(320), WindowManager.LayoutParams.WRAP_CONTENT, true);
        panelLp.gravity = Gravity.CENTER;
        wm.addView(panel, panelLp);
        nativeLog("panel added");

        fab.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                menuOpen = !menuOpen;
                panel.setVisibility(menuOpen ? View.VISIBLE : View.GONE);
                fab.setText(menuOpen ? "CLOSE" : "MENU");
                try { wm.updateViewLayout(panel, panelLp); } catch (Throwable ignored) {}
            }
        });

        attached = true;
        ui.post(ticker);
        Toast.makeText(activity, "AU CHEAT OK — press MENU", Toast.LENGTH_LONG).show();
        nativeLog("ATTACH OK type=" + fabLp.type);
    }

    /** LGL activity style: TYPE_APPLICATION. Fallback overlay if permitted. */
    private static WindowManager.LayoutParams params(int w, int h, boolean touchable) {
        int type = WindowManager.LayoutParams.TYPE_APPLICATION;
        try {
            if (Build.VERSION.SDK_INT >= 23 &&
                    Settings.canDrawOverlays(activity)) {
                type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
            }
        } catch (Throwable ignored) {}

        int flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
                | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
        if (!touchable) {
            flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        } else {
            flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                    | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
        }

        WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                w, h, type, flags, PixelFormat.TRANSLUCENT);
        lp.format = PixelFormat.TRANSLUCENT;
        lp.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN;
        return lp;
    }

    private static ScrollView buildPanel(Context ctx) {
        ScrollView scroll = new ScrollView(ctx);
        LinearLayout box = new LinearLayout(ctx);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(dp(16), dp(16), dp(16), dp(16));
        box.setBackgroundColor(0xF014141C);

        TextView title = new TextView(ctx);
        title.setText("Among Us Internal\n2026.6.5  |  " + Build.VERSION.SDK_INT);
        title.setTextColor(Color.WHITE);
        title.setTextSize(16f);
        box.addView(title);

        box.addView(chk("Player ESP", nativeGetEsp(), new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetEsp(c); }
        }));
        box.addView(chk("Murder ESP", nativeGetMurderEsp(), new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetMurderEsp(c); }
        }));
        box.addView(chk("Boxes", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetBox(c); }
        }));
        box.addView(chk("Snaplines", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetLine(c); }
        }));
        box.addView(chk("Names / Roles", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetName(c); }
        }));

        scroll.addView(box);
        return scroll;
    }

    private static CheckBox chk(String text, boolean on, CompoundButton.OnCheckedChangeListener l) {
        CheckBox c = new CheckBox(activity);
        c.setText(text);
        c.setTextColor(Color.WHITE);
        c.setChecked(on);
        c.setOnCheckedChangeListener(l);
        return c;
    }

    private static int dp(int v) {
        return Math.round(v * activity.getResources().getDisplayMetrics().density);
    }

    public static class EspView extends View {
        private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint text = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final float[] buf = new float[16 * 8];

        public EspView(Context ctx) {
            super(ctx);
            setWillNotDraw(false);
            setBackgroundColor(Color.TRANSPARENT);
            text.setTextSize(32f);
            text.setShadowLayer(4f, 1f, 1f, Color.BLACK);
        }

        @Override
        protected void onSizeChanged(int w, int h, int oldw, int oldh) {
            super.onSizeChanged(w, h, oldw, oldh);
            try { nativeSetViewSize(w, h); } catch (Throwable ignored) {}
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            int n;
            try { n = nativeEspCount(); } catch (Throwable t) { return; }
            if (n <= 0) return;
            if (n > 16) n = 16;
            try { nativeEspFill(buf); } catch (Throwable t) { return; }
            float midX = getWidth() * 0.5f;
            float baseY = getHeight() * 0.85f;
            for (int i = 0; i < n; i++) {
                int o = i * 8;
                float sx = buf[o], top = buf[o + 1], bot = buf[o + 2];
                int col = Color.argb(c(buf[o + 6]), c(buf[o + 3]), c(buf[o + 4]), c(buf[o + 5]));
                if (buf[o + 7] < 0.5f) continue;
                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth(buf[o + 7] > 1.5f ? 7f : 4f);
                paint.setColor(col);
                float hw = Math.max(20f, (bot - top) * 0.35f);
                canvas.drawRect(sx - hw, top, sx + hw, bot, paint);
                paint.setStrokeWidth(2.5f);
                canvas.drawLine(midX, baseY, sx, bot, paint);
                try {
                    String lab = nativeEspLabel(i);
                    if (lab != null && lab.length() > 0) {
                        text.setColor(col);
                        canvas.drawText(lab, sx - text.measureText(lab) * 0.5f, top - 10f, text);
                    }
                } catch (Throwable ignored) {}
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
