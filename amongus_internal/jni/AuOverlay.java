package au.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

/**
 * Overlay attached to Activity content view — works with GLES and Vulkan.
 */
public class AuOverlay {
    private static Activity activity;
    private static PassThroughRoot root;
    private static EspView espView;
    private static Button fab;
    private static ScrollView panel;
    private static boolean menuOpen = false;
    private static final Handler ui = new Handler(Looper.getMainLooper());
    private static final Runnable ticker = new Runnable() {
        @Override public void run() {
            if (espView != null) espView.postInvalidate();
            if (root != null) ui.postDelayed(this, 16);
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
                }
            }
        });
    }

    private static void attachLocked() {
        if (root != null || activity == null) return;
        Context ctx = activity;

        root = new PassThroughRoot(ctx);
        root.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));

        espView = new EspView(ctx);
        espView.setClickable(false);
        root.addView(espView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));

        fab = new Button(ctx);
        fab.setText("MENU");
        fab.setTextColor(Color.WHITE);
        fab.setTextSize(16f);
        fab.setAllCaps(true);
        fab.setBackgroundColor(0xF0E53935);
        fab.setPadding(dp(ctx, 18), dp(ctx, 18), dp(ctx, 18), dp(ctx, 18));
        FrameLayout.LayoutParams flp = new FrameLayout.LayoutParams(
                dp(ctx, 88), dp(ctx, 88));
        flp.gravity = Gravity.START | Gravity.CENTER_VERTICAL;
        flp.leftMargin = dp(ctx, 12);
        root.addView(fab, flp);
        root.setInteractive(fab);

        panel = buildPanel(ctx);
        panel.setVisibility(View.GONE);
        FrameLayout.LayoutParams plp = new FrameLayout.LayoutParams(
                dp(ctx, 300), ViewGroup.LayoutParams.WRAP_CONTENT);
        plp.gravity = Gravity.CENTER;
        root.addView(panel, plp);
        root.setPanel(panel);

        fab.setOnClickListener(new View.OnClickListener() {
            @Override public void onClick(View v) {
                menuOpen = !menuOpen;
                panel.setVisibility(menuOpen ? View.VISIBLE : View.GONE);
                fab.setText(menuOpen ? "CLOSE" : "MENU");
            }
        });

        ViewGroup content = activity.findViewById(android.R.id.content);
        if (content == null) {
            throw new IllegalStateException("android.R.id.content missing");
        }
        content.addView(root);
        ui.post(ticker);
    }

    private static ScrollView buildPanel(Context ctx) {
        ScrollView scroll = new ScrollView(ctx);
        LinearLayout box = new LinearLayout(ctx);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(dp(ctx, 16), dp(ctx, 16), dp(ctx, 16), dp(ctx, 16));
        box.setBackgroundColor(0xF014141C);

        TextView title = new TextView(ctx);
        title.setText("Among Us Internal\n2026.6.5 / Kitty");
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

    /** Root that only steals touches on FAB / open panel. */
    public static class PassThroughRoot extends FrameLayout {
        private View fabRef;
        private View panelRef;

        public PassThroughRoot(Context ctx) { super(ctx); }
        public void setInteractive(View fab) { fabRef = fab; }
        public void setPanel(View panel) { panelRef = panel; }

        private boolean hit(View v, MotionEvent ev) {
            if (v == null || v.getVisibility() != VISIBLE) return false;
            int[] loc = new int[2];
            v.getLocationOnScreen(loc);
            float x = ev.getRawX();
            float y = ev.getRawY();
            return x >= loc[0] && x <= loc[0] + v.getWidth()
                    && y >= loc[1] && y <= loc[1] + v.getHeight();
        }

        @Override
        public boolean dispatchTouchEvent(MotionEvent ev) {
            if (hit(fabRef, ev) || hit(panelRef, ev)) {
                return super.dispatchTouchEvent(ev);
            }
            return false;
        }
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
            text.setColor(Color.WHITE);
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
