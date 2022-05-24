//import { css as cssFn, ThemedCssFunction } from 'styled-components';

//const css = cssFn as unknown as ThemedCssFunction<never>;

const pxToRem = (px: number) => `${px / 16}rem`

const theme = {
  rem: pxToRem,
  colors: {
    red: 'red',
    primary: 'black',
  },
  spacing: {},
  typography: {},
  borderRadius: pxToRem(5),
}

export type Theme = typeof theme

export type Color = keyof Theme['colors']
export type Spacing = keyof Theme['spacing']

export default theme
