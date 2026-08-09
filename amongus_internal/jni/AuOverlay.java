package au.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Floating menu via WindowManager.TYPE_APPLICATION_PANEL (above Unity SurfaceView).
 */
public class AuOverlay {
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
    private static final Handler ui = new Handler(Looper.getMainLooper());
    private static final Runnable ticker = new Runnable() {
        @Override public void run() {
            if (espView != null) espView.postInvalidate();
            if (attached) ui.postDelayed(this, 16);
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

    public static void start(final Activity act) {
        if (act == null) return;
        activity = act;
        ui.post(new Runnable() {
            @Override public void run() {
                try {
                    attachLocked();
                } catch (Throwable t) {
                    t.printStackTrace();
                    try {
                        Toast.makeText(act, "AU overlay FAIL: " + t.getMessage(), Toast.LENGTH_LONG).show();
                    } catch (Throwable ignored) {}
                }
            }
        });
    }

    private static void attachLocked() {
        if (attached || activity == null) return;

        wm = activity.getWindowManager();
        Context ctx = activity;

        // ESP fullscreen (not touchable)
        espView = new EspView(ctx);
        espLp = baseParams(
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT);
        espLp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        espLp.gravity = Gravity.TOP | Gravity.START;
        setToken(espLp);
        wm.addView(espView, espLp);

        // FAB — big red button
        fab = new Button(ctx);
        fab.setText("MENU");
        fab.setTextColor(Color.WHITE);
        fab.setTextSize(18f);
        fab.setAllCaps(true);
        fab.setBackgroundColor(0xF0E53935);
        fabLp = baseParams(dp(ctx, 96), dp(ctx, 96));
        fabLp.gravity = Gravity.START | Gravity.CENTER_VERTICAL;
        fabLp.x = dp(ctx, 16);
        fabLp.y = 0;
        setToken(fabLp);
        wm.addView(fab, fabLp);

        // Panel
        panel = buildPanel(ctx);
        panelLp = baseParams(dp(ctx, 320), WindowManager.LayoutParams.WRAP_CONTENT);
        panelLp.gravity = Gravity.CENTER;
        setToken(panelLp);
        panel.setVisibility(View.GONE);
        wm.addView(panel, panelLp);

        fab.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                menuOpen = !menuOpen;
                panel.setVisibility(menuOpen ? View.VISIBLE : View.GONE);
                fab.setText(menuOpen ? "CLOSE" : "MENU");
                try {
                    wm.updateViewLayout(panel, panelLp);
                } catch (Throwable ignored) {}
            }
        });

        attached = true;
        ui.post(ticker);
        Toast.makeText(activity, "AU CHEAT OK — tap MENU", Toast.LENGTH_LONG).show();
    }

    private static void setToken(WindowManager.LayoutParams lp) {
        try {
            View decor = activity.getWindow().getDecorView();
            lp.token = decor.getWindowToken();
        } catch (Throwable ignored) {}
    }

    private static WindowManager.LayoutParams baseParams(int w, int h) {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                w,
                h,
                WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                        | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
                PixelFormat.TRANSLUCENT);
        lp.format = PixelFormat.TRANSLUCENT;
        return lp;
    }

    private static ScrollView buildPanel(Context ctx) {
        ScrollView scroll = new ScrollView(ctx);
        LinearLayout box = new LinearLayout(ctx);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(dp(ctx, 16), dp(ctx, 16), dp(ctx, 16), dp(ctx, 16));
        box.setBackgroundColor(0xF014141C);

        TextView title = new TextView(ctx);
        title.setText("Among Us Internal\n2026.6.5");
        title.setTextColor(Color.WHITE);
        title.setTextSize(16f);
        box.addView(title);

        box.addView(chk(ctx, "Player ESP", nativeGetEsp(), new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetEsp(c); }
        }));
        box.addView(chk(ctx, "Murder ESP", nativeGetMurderEsp(), new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetMurderEsp(c); }
        }));
        box.addView(chk(ctx, "Boxes", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetBox(c); }
        }));
        box.addView(chk(ctx, "Snaplines", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetLine(c); }
        }));
        box.addView(chk(ctx, "Names / Roles", true, new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton b, boolean c) { nativeSetName(c); }
        }));

        scroll.addView(box);
        return scroll;
    }

    private static CheckBox chk(Context ctx, String text, boolean on,
                                CompoundButton.OnCheckedChangeListener l) {
        CheckBox c = new CheckBox(ctx);
        c.setText(text);
        c.setTextColor(Color.WHITE);
        c.setChecked(on);
        c.setOnCheckedChangeListener(l);
        return c;
    }

    private static int dp(Context ctx, int v) {
        return Math.round(v * ctx.getResources().getDisplayMetrics().density);
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
        public boolean onTouchEvent(MotionEvent event) {
            return false;
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
                float sx = buf[o];
                float top = buf[o + 1];
                float bot = buf[o + 2];
                int col = Color.argb(
                        clamp255(buf[o + 6] * 255f),
                        clamp255(buf[o + 3] * 255f),
                        clamp255(buf[o + 4] * 255f),
                        clamp255(buf[o + 5] * 255f));
                float flags = buf[o + 7];
                if (flags < 0.5f) continue;

                paint.setStyle(Paint.Style.STROKE);
                paint.setStrokeWidth(flags > 1.5f ? 7f : 4f);
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

        private static int clamp255(float v) {
            if (v < 0f) return 0;
            if (v > 255f) return 255;
            return (int) v;
        }
    }
}
