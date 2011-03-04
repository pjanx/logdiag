-- Symbol names
local names_A =
{
	en = "Ammeter",
	cs = "Ampérmetr",
	sk = "Ampérmeter"
}

local names_V =
{
	en = "Voltmeter",
	cs = "Voltmetr",
	sk = "Voltmeter"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -2, 2, 2}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}}

-- Rendering
local render_A = function (cr)
	-- The circle
	cr.arc (0, 0, 2, 0, math.pi * 2)

	-- The letter A
	cr.move_to (-0.4, 0.5)
	cr.line_to (0, -0.5)
	cr.line_to (0.4, 0.5)

	cr.move_to (-0.3, 0.25)
	cr.line_to (0.3, 0.25)

	cr.stroke ()
end

local render_V = function (cr)
	-- The circle
	cr.arc (0, 0, 2, 0, math.pi * 2)

	-- The letter V
	cr.move_to (-0.4, -0.5)
	cr.line_to (0, 0.5)
	cr.line_to (0.4, -0.5)

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("Ammeter",      names_A,   area, terminals, render_A)
logdiag.register ("Voltmeter",    names_V,   area, terminals, render_V)


