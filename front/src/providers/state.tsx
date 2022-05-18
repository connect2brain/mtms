import create from 'zustand';
import { devtools } from 'zustand/middleware';

type Store = {
  asd: string;
  setAsd: (asd: string) => void;
}

const useStore = create<Store>(
  devtools((set) => ({
    asd: 'toimii',
    setAsd: asd => set({ asd })
  })),
);