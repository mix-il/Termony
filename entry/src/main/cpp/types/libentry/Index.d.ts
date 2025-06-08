export const run: () => void;
export const read: () => ArrayBuffer | undefined;
export const send: (content: ArrayBuffer) => void;
export const createSurface: (id: BigInt) => void;
export const destroySurface: (id: BigInt) => void;