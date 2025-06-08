export const run: () => void;
export const send: (content: ArrayBuffer) => void;
export const createSurface: (id: BigInt) => void;
export const destroySurface: (id: BigInt) => void;
export const resizeSurface: (id: BigInt, width: number, height: number) => void;