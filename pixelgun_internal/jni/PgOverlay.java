package pg.overlay;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;

/** ESP overlay — draws boxes/names/tracers from native (stanuwu ModuleESP style). */
public class PgOverlay {
    private static Activity activity;
    private static WindowManager wm;
    private static EspView esp;
    private static boolean attached;
    private static int tries;
    private static final Handler ui = new Handler(Looper.getMainLooper());

    public static native int nativeEspCount();
    public static native int nativeEspFill(float[] buf); // sx,sy,w2,h2,enemy,dist,nameIdx...
    public static native String nativeEspName(int i);
    public static native void nativeSetEsp(boolean v);
    public static native boolean nativeGetEsp();
    public static native String nativeStatus();

    public static void start(Activity act) {
        activity = act;
        ui.post(new Runnable() { public void run() { tryAttach(); } });
    }

    private static void tryAttach() {
        if (attached || activity == null) return;
        tries++;
        try {
            if (activity.getWindow().getDecorView().getWindowToken() == null && tries < 80) {
                ui.postDelayed(new Runnable() { public void run() { tryAttach(); } }, 200);
                return;
            }
            Context ctx = activity;
            wm = activity.getWindowManager();
            esp = new EspView(ctx);
            WindowManager.LayoutParams lp = new WindowManager.LayoutParams(
                    WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.MATCH_PARENT,
                    WindowManager.LayoutParams.TYPE_APPLICATION,
                    WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                            | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                            | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN
                            | WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED,
                    PixelFormat.TRANSLUCENT);
            lp.gravity = Gravity.TOP | Gravity.START;
            wm.addView(esp, lp);
            attached = true;
            ui.post(ticker);
        } catch (Throwable t) {
            if (tries < 80) ui.postDelayed(new Runnable() { public void run() { tryAttach(); } }, 300);
        }
    }

    private static final Runnable ticker = new Runnable() {
        public void run() {
            if (esp != null) esp.invalidate();
            if (attached) ui.postDelayed(this, 16);
        }
    };

    public static class EspView extends View {
        private final Paint box = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint text = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint line = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final float[] buf = new float[64 * 8];

        public EspView(Context c) {
            super(c);
            setBackgroundColor(Color.TRANSPARENT);
            box.setStyle(Paint.Style.STROKE);
            box.setStrokeWidth(3f);
            line.setStrokeWidth(2f);
            text.setTextSize(28f);
            text.setFakeBoldText(true);
            text.setShadowLayer(3f, 0, 1f, 0xCC000000);
        }

        @Override protected void onDraw(Canvas c) {
            int n;
            try { n = nativeEspFill(buf); } catch (Throwable t) { return; }
            if (n <= 0) return;
            float H = getHeight();
            float midX = getWidth() * 0.5f;
            float baseY = H * 0.92f;

            // status
            try {
                String st = nativeStatus();
                if (st != null) {
                    text.setColor(0xFFE8EEF7);
                    text.setTextSize(22f);
                    c.drawText(st, 24, 48, text);
                    text.setTextSize(28f);
                }
            } catch (Throwable ignored) {}

            for (int i = 0; i < n; i++) {
                int o = i * 8;
                float sx = buf[o];
                float syUnity = buf[o + 1];
                float w2 = buf[o + 2];
                float h2 = buf[o + 3];
                boolean enemy = buf[o + 4] > 0.5f;
                float dist = buf[o + 5];
                // flip Y like stanuwu: window_bottom - screen.y
                float sy = H - syUnity;

                int col = enemy ? 0xFFFF2020 : 0xFF2080FF;
                box.setColor(col);
                line.setColor(col);
                text.setColor(col);

                c.drawRect(sx - w2, sy - h2, sx + w2, sy + h2, box);
                if (enemy) c.drawLine(midX, baseY, sx, sy + h2, line);

                String name = "";
                try { name = nativeEspName(i); } catch (Throwable ignored) {}
                if (name == null) name = "";
                String label = name;
                if (dist > 0) label = name + " [" + (int) dist + "m]";
                float tw = text.measureText(label);
                c.drawText(label, sx - tw * 0.5f, sy - h2 - 8f, text);
            }
        }
    }
}
