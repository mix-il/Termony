import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import { taskpool } from '@kit.ArkTS';
import { util } from '@kit.ArkTS';
import fs from '@ohos.file.fs';

const DOMAIN = 0x0000;

testNapi.run();

class MyXComponentController extends XComponentController {
  onSurfaceCreated(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceCreated surfaceId: %{public}s', surfaceId);
    testNapi.createSurface(BigInt(surfaceId));
  }

  onSurfaceChanged(surfaceId: string, rect: SurfaceRect): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceChanged surfaceId: %{public}s rect: %{public}s', surfaceId, JSON.stringify(rect));
    testNapi.resizeSurface(BigInt(surfaceId), rect.surfaceWidth, rect.surfaceHeight);
  }

  onSurfaceDestroyed(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceDestroyed surfaceId: %{public}s', surfaceId);
    testNapi.destroySurface(BigInt(surfaceId))
  }
}

@Entry
@Component
struct Index {
  @State leftCtrlPressed: boolean = false;
  @State touchState: Map<number, number> = new Map();
  scroller: Scroller = new Scroller();
  xComponentController: XComponentController = new MyXComponentController();

  build() {
    Row() {
      Column() {
        XComponent({
          type: XComponentType.SURFACE,
          controller: this.xComponentController
        })
      }
      .width('100%')
    }
    .height('100%')
    .onTouch((event: TouchEvent) => {
      // hilog.info(DOMAIN, 'testTag', 'Got touch: %{public}s', JSON.stringify(event));
      for (let touch of event.touches) {
        if (touch.type === TouchType.Down) {
          this.touchState.set(touch.id, touch.y);
        } else if (touch.type === TouchType.Move) {
          // we use pixels, convert from vp to px
          let offset = vp2px(this.touchState.get(touch.id) as number - touch.y);
          this.touchState.set(touch.id, touch.y);
          // hilog.info(DOMAIN, 'testTag', 'Got touch offset: %{public}d', offset);
          testNapi.scroll(offset);
        }
      }
    })
    .onKeyEvent((event: KeyEvent) => {
      hilog.info(DOMAIN, 'testTag', 'Got key: %{public}s', JSON.stringify(event));
      if (event.type === KeyType.Down) {
        if (this.leftCtrlPressed && event.unicode as number >= 97 && event.unicode as number <= 122) {
          // Ctrl-A to Ctrl-Z
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          view[0] = event.unicode as number - 97 + 1; // ^A is 0x1
          testNapi.send(buffer);
        } else if (event.keyText === "KEYCODE_EQUALS") {
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          // workaround
          if (event.unicode === 0x3d) {
            view[0] = 0x2b;
          } else {
            view[0] = 0x3d;
          }
          testNapi.send(buffer);
        } else if (event.unicode !== 0) {
          let textEncoder = util.TextEncoder.create('utf-8');
          let encodeResult = textEncoder.encode(String.fromCharCode(event.unicode as number)); 
          testNapi.send(encodeResult.buffer);
        } else {
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          if (event.keyText === "KEYCODE_SPACE") {
            view[0] = 0x20;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_ENTER") {
            view[0] = 0x0d;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_DEL") {
            view[0] = 0x7f;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_TAB") {
            view[0] = 0x09;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_ESCAPE") {
            view[0] = 0x1b;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_CTRL_LEFT") {
            this.leftCtrlPressed = true;
          } else if (event.keyText === "KEYCODE_DPAD_UP") {
            let buffer = new ArrayBuffer(3);
            let view = new Uint8Array(buffer);
            view[0] = 0x1b;
            view[1] = 0x5b;
            view[2] = 0x41;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_DPAD_DOWN") {
            let buffer = new ArrayBuffer(3);
            let view = new Uint8Array(buffer);
            view[0] = 0x1b;
            view[1] = 0x5b;
            view[2] = 0x42;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_DPAD_RIGHT") {
            let buffer = new ArrayBuffer(3);
            let view = new Uint8Array(buffer);
            view[0] = 0x1b;
            view[1] = 0x5b;
            view[2] = 0x43;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_DPAD_LEFT") {
            let buffer = new ArrayBuffer(3);
            let view = new Uint8Array(buffer);
            view[0] = 0x1b;
            view[1] = 0x5b;
            view[2] = 0x44;
            testNapi.send(buffer);
          }
        }
      } else if (event.type === KeyType.Up) {
        if (event.keyText === "KEYCODE_CTRL_LEFT") {
          this.leftCtrlPressed = false;
        }
      }
    })
  }
}
