-- Symbol names
local names =
{
	en = "Diode",
	cs = "Dioda",
	sk = "Dióda",
	pl = "Dioda",
	de = "Diode"
}

local names_zener =
{
	en = "Zener diode",
	cs = "Zenerova dioda",
	sk = "Zenerova dióda",
	pl = "Dioda Zenera",
	de = "Zenerdiode"
}

local names_led =
{
	en = "Light-emitting diode",
	cs = "Svítivá dioda",
	sk = "Svietivá dióda",
	pl = "Dioda świecąca",
	de = "Lichtemittierende Diode"
}

local names_photo =
{
	en = "Photodiode",
	cs = "Fotodioda",
	sk = "Fotodióda",
	pl = "Fotodioda",
	de = "Fotodiode"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area     = {-2, -1, 2, 1}
local area_rad = {-2, -2.5, 2, 1}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render = function (cr)
	-- The triangle
	cr:move_to (-1, -1)
	cr:line_to (1, 0)
	cr:line_to (-1, 1)
	cr:close_path ()

	-- The vertical line
	cr:move_to (1, 1)
	cr:line_to (1, -1)

	-- The terminals
	cr:move_to (-2, 0)
	cr:line_to (2, 0)

	cr:stroke ()
end

local render_zener = function (cr)
	render (cr)

	cr:move_to (1, 1)
	cr:line_to (0.5, 1)

	cr:stroke ()
end

local render_arrow = function (cr)
	cr:move_to (0, 0)
	cr:line_to (0, -1.5)

	cr:stroke ()

	cr:move_to (-0.3, -0.7)
	cr:line_to (0, -1.5)
	cr:line_to (0.3, -0.7)
	cr:close_path ()

	cr:fill ()
end

local render_radiation = function (cr)
	cr:save ()
	cr:translate (-0.4, 0)
	render_arrow (cr)
	cr:restore ()

	cr:save ()
	cr:translate (0.4, 0)
	render_arrow (cr)
	cr:restore ()
end

local render_led = function (cr)
	render (cr)

	cr:save ()
	cr:translate (-0.3, -1.0)
	cr:rotate (math.atan2 (1, 1))

	render_radiation (cr)

	cr:restore ()
end

local render_photo = function (cr)
	render (cr)

	cr:save ()
	cr:translate (0.75, -2.05)
	cr:rotate (math.atan2 (-1, -1))

	render_radiation (cr)

	cr:restore ()
end

-- Register the symbol
logdiag.register ("Diode",      names,       area,     terminals, render)
logdiag.register ("DiodeZener", names_zener, area,     terminals, render_zener)
logdiag.register ("DiodeLED",   names_led,   area_rad, terminals, render_led)
logdiag.register ("DiodePhoto", names_photo, area_rad, terminals, render_photo)


