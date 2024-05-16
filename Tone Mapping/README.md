# DDK - DRT Developer Toolkit

## Why This Exists

As DRT's become more popular, and as people become aware that there are different ways to shepherd your image from the camera to the screen, there has been a lot of debate as to what is the best method.

In my opinion, a DRT has to do 4 things:

- Compress the dynamic range of the luminosity so it can be displayed
- Compress the gamut to what the display can handle
- Apply a 3x3 matrix to move from one color gamut to another
- Apply "inverse display encoding" or "inverse gamma correction" so it is encoded as the display expects

The last two of those operations require no creativity, and in fact, don't really allow for any. The equations and values are published in white papers, and a DRT simply implements them. The first two however, are not specified and are purely preferential. There are many different approaches to tone and gamut mapping, and a lot of debate on what is the ideal way to do this. I want to give my "hot take" on this question. There is no perfect DRT. Sure some may be better than others and some may be technically more accurate than others, but none of that matters. There is a best DRT for YOU, for YOUR PROJECT. Choosing the DRT is a creative choice, the look that it creates is part of YOUR LOOK.

So given that we are making a choice, even if we are not aware of it, the goal of this project is to give that option to the artist, to make those choices. At the very least to look at the different options and find out what they don't like. As opposed to a single DCTL which makes those choices for you, my idea here is to give you a toolkit of different tone and gamut compression techniques.

In my opinion, we have a pipeline that often looks like this:

IDT => Shot Level Adjustments => Macro Look => | => Tone Mapping => Gamut Mapping => Display Transform

and it seemed to me that people have arbitrarally drawn a line and said everything on this side is a "look" while everything on the other side of that vertical line is the DRT. I don't agree, or rather I would say that it is not a useful way to divide things. I want to divide things into The Preferential and the Technical.

If you are making a choice, touching a slider, checking a box, that is probably preferential. After all, you didn't have to do that, you probably decided to check that box or touch that slider because it gave you an image you preferred. Baselight's TCAM system kind of embraces this. The DRT should be as minimal and do as little as possible, the image is not expected to look good with just the DRT. In order to get an aesthetically pleasing image, you should apply a look upstream. I want to embrace that mentality. Let's find out how this works.

## How to use this

Given that we want to be able to exercise control over all the preferential portions of the pipeline, this system or framework allows you to add as many nodes as you would like before the CST. Your mission, if you choose to accept it, is to find the right combination of tone mapping operators and gamut mapping operators to give you the image you want. Once you find a combination that works for you, that matches your sensibilities, you can just continue to use it, or feel free to tweak it in the future.

Here are the "rules":

1. All the nodes I publish expect to receive XYZ/Linear images, they will return the same.
2. Typically you will want to put your tone mapping operators before your gamut mapping operators, but this is not required, if you prefer it the other way, that is up to you.
3. You will pipe the output of your last node into a Resolve CST which should expect XYZ/Linear
4. That CST should not have tone or gamut mapping turned on, it should purely implement the whitepaper for the space you are going to.
5. By leaving the math of inputs and outputs to Resolve's CST, the DCTL's can focus only on the operations they care about.
6. Each node should ideally only do one thing, it may apply corrections after doing its operation.
7. I haven't decided about OOTF, stay tuned as I learn more.

## What I have so far

### Tone Mapping Operators

- ACES RRT - You can do this on a per-channel basis or scaled via luminance, you can also change any parameter of the curve
- Drago - You can set the max luminance and the bias which controls how much compression is applied.
- Filmic - You can change any of the values that calculate the curve
- Hable Uncharted 2 - You can toggle between per channel and luminance scaled, you can control each of the parameters to the curve.
- Reinhard:
  - None - Does nothing
  - Y channel - Simple luminance based on the Y channel
  - Average - Average of the 3 channels using `0.2126 * value.x + 0.7152 * value.y + 0.0722 * value.z;`
  - Separately apply the reinhard tone mapping to each channel
  - Reinhard tone mapping after a value hits a certain threshold
  - Reinhard tone mapping using the max value of the x, y, z channels

### Gamut Mapping Operators

I haven't done any yet. Stay tuned.

DISCLAIMER:
I am just learning a lot of this stuff. This is my idea based on logic and kind of what I see everyone talking about. I see a lot of people copying the same input and output transforms, just so they can add one special preferential thing to their DRT, and then people have to kind of take it or leave it, all or nothing.

Not everyone is able to program their own DRT's, and I think that is a shame, hopefully, by being able to assemble your own, you can get a little closer to taking full control over your image.
