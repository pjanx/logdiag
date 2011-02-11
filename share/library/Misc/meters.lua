-- Symbol names
local names_A =
{
	en = "Ammeter",
	cs = "Ampérmetr"
}

local names_mA =
{
	en = "Milliammeter",
	cs = "Miliampérmetr"
}

local names_V =
{
	en = "Voltmeter",
	cs = "Voltmetr"
}

local names_ohm =
{
	en = "Ohmmeter",
	cs = "Ohmmetr"
}

local names_W =
{
	en = "Wattmeter",
	cs = "Wattmetr"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-3, -2, 3, 2}

-- Terminal points
local terminals = {{-3, 0}, {3, 0}}

-- Rendering
local render = function (cr, name)
	-- The circle
	cr.arc (0, 0, 2, 0, math.pi * 2)

	-- The contact
	cr.move_to (-3, 0)
	cr.line_to (-2, 0)

	cr.move_to (2, 0)
	cr.line_to (3, 0)

	cr.stroke ()

	cr.move_to (0, 0)
	cr.show_text (name)
end

local render_A = function (cr)
	render (cr, "A")
end

local render_mA = function (cr)
	render (cr, "mA")
end

local render_V = function (cr)
	render (cr, "V")
end

local render_ohm = function (cr)
	render (cr, "Ω")
end

local render_W = function (cr)
	render (cr, "W")
end

-- Register the symbols
logdiag.register ("Ammeter",      names_A,   area, terminals, render_A)
logdiag.register ("Milliammeter", names_mA,  area, terminals, render_mA)
logdiag.register ("Voltmeter",    names_V,   area, terminals, render_V)
logdiag.register ("Ohmmeter",     names_ohm, area, terminals, render_ohm)
logdiag.register ("Wattmeter",    names_W,   area, terminals, render_W)


