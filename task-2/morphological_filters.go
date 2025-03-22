package main

import (
	"image"
	"image/color"
	"sync"
)

type MorphologicalFilter struct {
	Size      int
	IsErosion bool
}

func (f MorphologicalFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	filtered := image.NewRGBA(bounds)
	offset := f.Size / 2
	var wg sync.WaitGroup
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		wg.Add(1)
		go func(y int) {
			defer wg.Done()
			for x := bounds.Min.X; x < bounds.Max.X; x++ {
				var minR, minG, minB uint32 = 65535, 65535, 65535
				var maxR, maxG, maxB uint32 = 0, 0, 0
				for ky := -offset; ky <= offset; ky++ {
					for kx := -offset; kx <= offset; kx++ {
						ix := x + kx
						iy := y + ky
						var r, g, b uint32
						if ix < bounds.Min.X || ix >= bounds.Max.X || iy < bounds.Min.Y || iy >= bounds.Max.Y {
							r, g, b = 0, 0, 0
						} else {
							r, g, b, _ = img.At(ix, iy).RGBA()
						}
						if f.IsErosion {
							minR = min(minR, r)
							minG = min(minG, g)
							minB = min(minB, b)
						} else {
							maxR = max(maxR, r)
							maxG = max(maxG, g)
							maxB = max(maxB, b)
						}
					}
				}
				var r, g, b uint32
				if f.IsErosion {
					r, g, b = minR, minG, minB
				} else {
					r, g, b = maxR, maxG, maxB
				}
				filtered.Set(x, y, color.RGBA{uint8(r >> 8), uint8(g >> 8), uint8(b >> 8), 255})
			}
		}(y)
	}
	wg.Wait()
	return filtered
}

func min(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}

func max(a, b uint32) uint32 {
	if a > b {
		return a
	}
	return b
}
