-- Symbol names
local names_ac =
{
	en = "AC voltage source",
	cs = "Střídavý zdroj napětí"
}

local names_dc =
{
	en = "DC voltage source",
	cs = "Stejnosměrný zdroj napětí"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area_ac = {-3, -0.5, 3, 0.5}
local area_dc = {-3, -1.25, 3, 0.5}

-- Terminal points
local terminals = {{-3, 0}, {3, 0}}

-- Rendering
local render = function (cr)
	-- The circles
	cr.arc (-2, 0, 0.3, 0, math.pi * 2)
	cr.new_sub_path ()
	cr.arc (2, 0, 0.3, 0, math.pi * 2)

	-- The terminals
	cr.move_to (-3, 0)
	cr.line_to (-2.3, 0)

	cr.move_to (2.3, 0)
	cr.line_to (3, 0)

	cr.stroke ()
end

local render_ac = function (cr)
	render (cr)

	-- The AC symbol
	cr.move_to (-1, 0.25)
	cr.curve_to (-0.4, -1.5, 0.4, 1.5, 1, -0.25)

	cr.stroke ()
end

local render_dc = function (cr)
	render (cr)

	-- The DC symbol
	cr.move_to (-1, -0.25)
	cr.line_to (1, -0.25)

	cr.move_to (-1, 0.25)
	cr.line_to (-0.2, 0.25)

	cr.move_to (0.2, 0.25)
	cr.line_to (1, 0.25)

	-- Polarity sign
	cr.move_to (2, -0.75)
	cr.line_to (2, -1.25)

	cr.move_to (1.75, -1)
	cr.line_to (2.25, -1)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("ACSource", names_ac, area_ac, terminals, render_ac)
logdiag.register ("DCSource", names_dc, area_dc, terminals, render_dc)


