package main

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"io"
	"log"
	"os"
	"strconv"

	"errors"

	"gioui.org/app"
	"gioui.org/io/event"
	"gioui.org/layout"
	"gioui.org/op"
	"gioui.org/op/clip"
	"gioui.org/op/paint"
	"gioui.org/unit"
	"gioui.org/widget"
	"gioui.org/widget/material"
	"gioui.org/x/explorer"
)

const className = "gio"
const brighthnessFactor = 1.2
const contrastFactor = 0.2
const gammaValue = 1.5

type ImageResult struct {
	Error  error
	Format string
	Image  image.Image
}

func getK(input string) (uint32, error) {
	k, err := strconv.ParseUint(input, 10, 32)
	if err != nil {
		return 0, err
	}
	if k < 2 {
		return 0, errors.New("k should be greater than 1")
	}
	return uint32(k), nil
}

func addFilter(filteredImage *FilteredImage, filter Filter) error {
	if filteredImage == nil {
		return errors.New("no image loaded")
	}
	filteredImage.AddFilter(filter)
	return nil
}

func drawSeparator(gtx layout.Context) layout.Dimensions {
	bounds := image.Rect(0, 5, gtx.Constraints.Max.X-5, 5)

	paint.FillShape(gtx.Ops, color.NRGBA{0, 0, 0, 255}, clip.Stroke{Path: clip.Rect(bounds).Path(), Width: 2}.Op())
	d := image.Point{Y: 10}
	return layout.Dimensions{Size: d}
}

func SaveImage(img image.Image, writer io.Writer) error {
	return png.Encode(writer, img)
}

func handleErrorDithering(img *FilteredImage, selected *widget.Enum, rIn, gIn, bIn *widget.Editor, options map[string]ErrorFilter) error {
	if img == nil {
		return errors.New("no image loaded")
	}

	selectedFilter, ok := options[selected.Value]
	if !ok {
		return errors.New("invalid filter selected")
	}

	kr, err := getK(rIn.Text())
	if err != nil {
		return fmt.Errorf("invalid kr %s", err.Error())
	}
	kg, err := getK(gIn.Text())
	if err != nil {
		return fmt.Errorf("invalid kg %s", err.Error())
	}
	kb, err := getK(bIn.Text())
	if err != nil {
		return fmt.Errorf("invalid kb %s", err.Error())
	}

	img.AddFilter(ErrorDithering{
		KR:     kr,
		KG:     kg,
		KB:     kb,
		Filter: selectedFilter,
	})
	return nil
}

func handleUniformQuantization(filteredImage *FilteredImage, krInput, kgInput, kbInput *widget.Editor) error {
	if filteredImage == nil {
		return errors.New("no image loaded")
	}

	kr, err := getK(krInput.Text())
	if err != nil {
		return fmt.Errorf("invalid kr %s", err.Error())
	}
	kg, err := getK(kgInput.Text())
	if err != nil {
		return fmt.Errorf("invalid kg %s", err.Error())
	}
	kb, err := getK(kbInput.Text())
	if err != nil {
		return fmt.Errorf("invalid kb %s", err.Error())
	}

	filteredImage.AddFilter(UniformQuantization{
		KR: kr,
		KG: kg,
		KB: kb,
	})

	return nil
}

func main() {
	go func() {
		window := new(app.Window)
		window.Option(app.Title(className))
		window.Option(app.MinSize(1920, 1080))

		err := run(window)
		if err != nil {
			log.Fatal(err)
		}
		os.Exit(0)
	}()
	app.Main()
}

func run(window *app.Window) error {
	theme := material.NewTheme()
	var ops op.Ops
	expl := explorer.NewExplorer(window)
	imgChan := make(chan ImageResult)
	events := make(chan event.Event)
	acks := make(chan struct{})
	var filteredImage *FilteredImage
	openButton := new(widget.Clickable)
	saveButton := new(widget.Clickable)
	inversionButton := new(widget.Clickable)
	brightnessButton := new(widget.Clickable)
	contrastButton := new(widget.Clickable)
	gammaButton := new(widget.Clickable)
	resetButton := new(widget.Clickable)
	loadKernelButton := new(widget.Clickable)
	saveKernelButton := new(widget.Clickable)
	applyKernelButton := new(widget.Clickable)
	erosionButton := new(widget.Clickable)
	dilationButton := new(widget.Clickable)
	grayScaleButton := new(widget.Clickable)
	errorDitheringButton := new(widget.Clickable)
	uniformQuantizationButton := new(widget.Clickable)
	ycbcrButton := new(widget.Clickable)
	convolutionEditor := NewConvolutionEditor()
	var errorMessage *string = new(string)
	originalImageScroll := widget.List{List: layout.List{Axis: layout.Horizontal}}
	filteredImageScroll := widget.List{List: layout.List{Axis: layout.Vertical}}

	filters := []struct {
		filter Filter
		button *widget.Clickable
	}{
		{filter: InversionFilter{}, button: inversionButton},
		{filter: BrightnessFilter{Factor: brighthnessFactor}, button: brightnessButton},
		{filter: ContrastFilter{Factor: contrastFactor}, button: contrastButton},
		{filter: GammaFilter{Gamma: gammaValue}, button: gammaButton},
		{filter: MorphologicalFilter{Size: 3, IsErosion: true}, button: erosionButton},
		{filter: MorphologicalFilter{Size: 3, IsErosion: false}, button: dilationButton},
		{filter: GrayscaleFilter{}, button: grayScaleButton},
	}

	kInput := new(widget.Editor)
	kInput.SetText("2")
	krInput := new(widget.Editor)
	krInput.SetText("2")
	kgInput := new(widget.Editor)
	kgInput.SetText("2")
	kbInput := new(widget.Editor)
	kbInput.SetText("2")

	filterSelect := new(widget.Enum)
	filterSelect.Value = "FloydSteinberg"

	filterOptions := map[string]ErrorFilter{
		"FloydSteinberg": FloydSteinbergFilter,
		"Burkes":         BurkesFilter,
		"Stucki":         StuckiFilter,
		"Sierra":         SierraFilter,
		"Atkinson":       AtkinsonFilter,
	}

	go func() {
		for {
			ev := window.Event()
			events <- ev
			<-acks
			if _, ok := ev.(app.DestroyEvent); ok {
				return
			}
		}
	}()
	for {
		select {
		case imgResult := <-imgChan:
			filteredImage = NewFilteredImage(imgResult.Image)
			window.Invalidate()
		case e := <-events:
			switch e := e.(type) {
			case app.DestroyEvent:
				return e.Err
			case app.FrameEvent:
				gtx := app.NewContext(&ops, e)

				if openButton.Clicked(gtx) {
					go func() {
						file, err := expl.ChooseFile("png", "jpeg", "jpg")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to open image: %v", err)
							return
						}
						defer file.Close()
						imgData, format, err := image.Decode(file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to decode image: %v", err)
							return
						}
						*errorMessage = ""
						imgChan <- ImageResult{Image: imgData, Format: format}
					}()
				}
				if saveButton.Clicked(gtx) {
					go func() {
						file, err := expl.CreateFile("image.png")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel file: %v", err)
							return
						}
						defer file.Close()
						err = SaveImage(filteredImage.FilteredImage, file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save image: %v", err)
							return
						}
						*errorMessage = ""
					}()
				}
				for _, f := range filters {
					if f.button.Clicked(gtx) {
						if err := addFilter(filteredImage, f.filter); err != nil {
							*errorMessage = err.Error()
							break
						}
						*errorMessage = ""
						window.Invalidate()
					}
				}
				if errorDitheringButton.Clicked(gtx) {
					err := handleErrorDithering(filteredImage, filterSelect, krInput, kgInput, kbInput, filterOptions)
					if err != nil {
						*errorMessage = err.Error()
					} else {
						*errorMessage = ""
					}
					window.Invalidate()
				}
				if uniformQuantizationButton.Clicked(gtx) {
					if err := handleUniformQuantization(filteredImage, krInput, kgInput, kbInput); err != nil {
						*errorMessage = err.Error()
					} else {
						*errorMessage = ""
					}
					window.Invalidate()
				}
				if ycbcrButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						filteredImage.AddFilter(YCbCrFilter)
						*errorMessage = ""
						window.Invalidate()
					}
				}
				if resetButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
					} else {
						*errorMessage = ""
						filteredImage.ResetFilters()
						window.Invalidate()
					}
				}
				if loadKernelButton.Clicked(gtx) {
					go func() {
						file, err := expl.ChooseFile("filter")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to open kernel file: %v", err)
							return
						}
						defer file.Close()
						filter, err := LoadConvolutionFilter(file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to load kernel: %v", err)
							return
						}
						convolutionEditor.LoadFilter(filter)
						*errorMessage = ""
					}()
				}
				if saveKernelButton.Clicked(gtx) {
					go func() {
						file, err := expl.CreateFile("filter.filter")
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel file: %v", err)
							return
						}
						defer file.Close()
						filter, err := convolutionEditor.GetFilter()
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel: %v", err)
							return
						}
						err = SaveConvolutionFilter(*filter, file)
						if err != nil {
							*errorMessage = fmt.Sprintf("Failed to save kernel: %v", err)
							return
						}
						*errorMessage = ""
					}()
				}
				if applyKernelButton.Clicked(gtx) {
					if filteredImage == nil {
						*errorMessage = "No image loaded"
						continue
					}
					err := convolutionEditor.UpdateKernel()
					if err != nil {
						*errorMessage = err.Error()
					} else {
						if convolutionEditor.ErrorMessage != "" {
							*errorMessage = convolutionEditor.ErrorMessage
						} else {
							*errorMessage = ""
							log.Printf("Applying kernel: %v", convolutionEditor.Kernel)
							filteredImage.AddFilter(ConvolutionFilter{Kernel: convolutionEditor.Kernel,
								Divisor: convolutionEditor.Divisor,
								Offset:  convolutionEditor.Offset,
								AnchorX: convolutionEditor.AnchorX,
								AnchorY: convolutionEditor.AnchorY})
						}
						window.Invalidate()
					}
				}

				layout.Flex{
					Axis:      layout.Horizontal,
					Spacing:   layout.SpaceEvenly,
					Alignment: layout.Middle,
				}.Layout(gtx,
					layout.Flexed(7, func(gtx layout.Context) layout.Dimensions {
						return layout.Flex{
							Axis:    layout.Horizontal,
							Spacing: layout.SpaceEvenly,
						}.Layout(gtx,
							layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
								return layout.Inset{Top: unit.Dp(10),
									Bottom: unit.Dp(10),
									Left:   unit.Dp(10),
									Right:  unit.Dp(10),
								}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
									return material.List(theme, &originalImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
										return material.List(theme, &filteredImageScroll).Layout(gtx, 1, func(gtx layout.Context, index int) layout.Dimensions {
											if filteredImage == nil || filteredImage.OriginalImage == nil {
												return layout.Dimensions{}
											} else {
												img := widget.Image{
													Src: paint.NewImageOp(filteredImage.OriginalImage),
													Fit: widget.Unscaled,
												}
												return img.Layout(gtx)
											}
										})
									})
								})
							}),
							layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
								return layout.Inset{Top: unit.Dp(10),
									Bottom: unit.Dp(10),
									Left:   unit.Dp(10),
									Right:  unit.Dp(10),
								}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
									return material.List(theme, &originalImageScroll).Layout(gtx, 1, func(gtx layout.Context, _ int) layout.Dimensions {
										return material.List(theme, &filteredImageScroll).Layout(gtx, 1, func(gtx layout.Context, _ int) layout.Dimensions {
											if filteredImage == nil || filteredImage.OriginalImage == nil {
												return layout.Dimensions{}
											} else {
												img := widget.Image{
													Src: paint.NewImageOp(filteredImage.FilteredImage),
													Fit: widget.Unscaled,
												}
												return img.Layout(gtx)
											}
										})
									})
								})
							}),
						)
					}),
					layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
						return layout.Flex{
							Axis:    layout.Vertical,
							Spacing: layout.SpaceEnd,
						}.Layout(gtx,
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									if *errorMessage != "" {
										return layout.Inset{Top: unit.Dp(10),
											Bottom: unit.Dp(10),
											Left:   unit.Dp(10),
											Right:  unit.Dp(10),
										}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
											caption := material.Caption(theme, *errorMessage)
											caption.Color = color.NRGBA{255, 0, 0, 255}
											return caption.Layout(gtx)
										})
									} else {
										return layout.Dimensions{}
									}
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, openButton, "Open").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, saveButton, "Save").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, inversionButton, "Invert").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, brightnessButton, "Brighten").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, contrastButton, "Contrast").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, gammaButton, "Gamma").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceEvenly,
									}.Layout(gtx,
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, erosionButton, "Erosion").Layout(gtx)
											})
										}),
										layout.Flexed(0.5, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Button(theme, dilationButton, "Dilation").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, resetButton, "Reset").Layout(gtx)
									})
								},
							),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, loadKernelButton, "Load Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, saveKernelButton, "Save Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, applyKernelButton, "Apply Kernel").Layout(gtx)
									})
								},
							),
							layout.Rigid(func(gtx layout.Context) layout.Dimensions {
								return convolutionEditor.Layout(gtx, theme)
							}),
							layout.Rigid(drawSeparator),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, grayScaleButton, "Grayscale").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis:    layout.Horizontal,
										Spacing: layout.SpaceBetween,
									}.Layout(gtx,
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Body1(theme, "KR:").Layout(gtx)
											})
										}),
										layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Editor(theme, krInput, "kr").Layout(gtx)
											})
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Body1(theme, "KG:").Layout(gtx)
											})
										}),
										layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Editor(theme, kgInput, "kg").Layout(gtx)
											})
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Body1(theme, "KB:").Layout(gtx)
											})
										}),
										layout.Flexed(1, func(gtx layout.Context) layout.Dimensions {
											return layout.Inset{Top: unit.Dp(5),
												Bottom: unit.Dp(5),
												Left:   unit.Dp(5),
												Right:  unit.Dp(5),
											}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
												return material.Editor(theme, kbInput, "kb").Layout(gtx)
											})
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Flex{
										Axis: layout.Vertical,
									}.Layout(gtx,
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.Body1(theme, "Select Filter:").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "FloydSteinberg", "Floyd-Steinberg").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Burkes", "Burkes").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Stucki", "Stucki").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Sierra", "Sierra").Layout(gtx)
										}),
										layout.Rigid(func(gtx layout.Context) layout.Dimensions {
											return material.RadioButton(theme, filterSelect, "Atkinson", "Atkinson").Layout(gtx)
										}),
									)
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, errorDitheringButton, "Error Dithering").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, uniformQuantizationButton, "Uniform Quantization").Layout(gtx)
									})
								},
							),
							layout.Rigid(
								func(gtx layout.Context) layout.Dimensions {
									return layout.Inset{Top: unit.Dp(5),
										Bottom: unit.Dp(5),
										Left:   unit.Dp(5),
										Right:  unit.Dp(5),
									}.Layout(gtx, func(gtx layout.Context) layout.Dimensions {
										return material.Button(theme, ycbcrButton, "YCbCr").Layout(gtx)
									})
								},
							),
						)
					}),
				)

				e.Frame(gtx.Ops)
			}
			acks <- struct{}{}
		}
	}
}
