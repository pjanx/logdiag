-- Symbol names
local names_plus =
{
	en = "Plus sign",
	cs = "Znaménko plus",
	sk = "Znamienko plus",
	pl = "Znaczek plus",
	de = "Pluszeichen"
}

local names_minus =
{
	en = "Minus sign",
	cs = "Znaménko mínus",
	sk = "Znamienko mínus",
	pl = "Znaczek minus",
	de = "Minuszeichen"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-0.5, -0.5, 0.5, 0.5}

-- Terminal points
local terminals = {}

-- Rendering
local render_plus = function (cr)
	-- The plus sign
	cr:move_to (0, -0.4)
	cr:line_to (0, 0.4)

	cr:move_to (-0.4, 0)
	cr:line_to (0.4, 0)

	cr:stroke ()
end

local render_minus = function (cr)
	-- The minus sign
	cr:move_to (-0.4, 0)
	cr:line_to (0.4, 0)

	cr:stroke ()
end

-- Register the symbols
logdiag.register ("SignPlus",  names_plus,  area, terminals, render_plus)
logdiag.register ("SignMinus", names_minus, area, terminals, render_minus)


