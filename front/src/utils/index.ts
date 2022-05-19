const expand = (obj: any) =>
  obj ? Object.keys(obj)
    .map((key) => obj[key].toFixed(3))
    .join(', ') : ''

export default expand